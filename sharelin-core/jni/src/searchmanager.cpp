/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009  Andrey Stroganov <savthe@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "searchmanager.hpp"
#include "g2net.hpp"
#include "hubcache.hpp"
#include "searcher.hpp"
#include "g2packet.hpp"
#include <boost/bind.hpp>
#include <assert.h>
#include "system.hpp"
#include "searchdefs.hpp"
#include <functional>

namespace {

const std::time_t REST_TIME = 1 * 60;
const uint QKEY_DURATION = 60 * 60 * 1;
const uint SEARCHES_PER_ROUND = 5;
const uint MIN_QKEY_REQUESTS = 15;

inline std::time_t RatedSince(std::time_t now, boost::shared_ptr<Search::Searcher> p)
{
	if(p->CountResults() > 4000) return 0;
	return p->Rate() * (now - p->SearchedTime());
}

struct NotSearched: public std::unary_function<Ip::Address, bool>
{
	typedef Search::Manager::SearcherPtr SearcherPtr;

	NotSearched(SearcherPtr p): pSearcher(p) {}

	bool operator()(const G2::HubInfo& hub) const
	{
		return !pSearcher->Searched(hub.GetAddress());
	}

	SearcherPtr pSearcher;
};

}

namespace Search {

Manager::Manager():
	periodSearch_(Time::Seconds(1)),
	periodQueryKey_(Time::Milliseconds(500)),
	needKeys_(0)
{
}

void Manager::Start()
{
	System::GetNetwork()->AddHandler(G2::QKA, boost::bind(&Manager::OnQKA, this, _1, _2, _3));
	System::GetNetwork()->AddHandler(G2::QH2, boost::bind(&Manager::OnQH2, this, _1, _2, _3));
	System::GetNetwork()->AddHandler(G2::QA, boost::bind(&Manager::OnQA, this, _1, _2, _3));
}

unsigned Manager::CountUserSearches() const
{
	unsigned n = 0;

	for(SearchersContainer::const_iterator i = searchers_.begin(); i != searchers_.end(); ++i)
		if(i->second->GetID()) ++n;

	return n;
}

void Manager::OnQA(const G2::Packet& pk, Ip::Endpoint endpoint, Ip::Protocol::Type proto)
{
	Ip::Endpoint remote;
	if(proto == Ip::Protocol::UDP)
		remote = endpoint;

	uint tryAfter = 0;

	G2::Reader r(pk);
	while(r.Next())
	{
		switch(r.Type())
		{
			case G2::D:
			{
				Ip::Endpoint ep(r.Pod<uint32>(), r.Pod<uint16>(4));
				
				G2::HubInfo hub = System::GetHubCache()->Find(ep.address);
				if(hub.Empty()) hub.SetEndpoint(ep);

				hub.Seen(System::Now());
				hub.SearchAfter(System::Now() + G2::FOREIGN_HUB_SEARCH_PERIOD);
				System::GetHubCache()->Update(hub);
				break;
			}
			case G2::S:
			{
				Ip::Endpoint ep(r.Pod<uint32>(), r.Pod<uint16>(4));
				System::GetHubCache()->Touch(ep);
				break;
			}
			case G2::FR:
			{
				remote = Ip::Endpoint(r.Pod<uint32>(), r.Pod<uint16>(4));
				break;
			}
			case G2::RA:
			{
				tryAfter = r.BytesLeft() == 2 ? r.Pod<uint16>() : r.Pod<uint32>();
				break;
			}
		}
	}

	if(!remote.Empty())
	{
		G2::HubInfo hub = System::GetHubCache()->Find(remote.address);
		if(hub.Empty()) hub.SetEndpoint(remote);
		if(tryAfter) hub.SearchAfter(System::Now() + tryAfter);
		hub.Seen(System::Now());
		System::GetHubCache()->Update(hub);
	}
}

void Manager::OnQH2(const G2::Packet& qh2, Ip::Endpoint endpoint, Ip::Protocol::Type proto)
{
	GUID guid;
	G2::Reader r(qh2);
	r.Skip(1);
	r.Read(guid.Bytes(), 16);

	SearchersContainer::iterator i = searchers_.find(guid);
	if(i != searchers_.end())
		i->second->OnHit(qh2, endpoint);
}

void Manager::OnQKA(const G2::Packet& qka, Ip::Endpoint endpoint, Ip::Protocol::Type proto)
{
	//std::cout << "GOT QKA from " << endpoint.ToString() << " need: " << requestKeys_ << std::endl;
	if(needKeys_ == 0) return;

	uint32 qkey = 0;
	Ip::Endpoint proxy;
	Ip::Endpoint remote;

	if(proto == Ip::Protocol::UDP)
	{	
		remote = endpoint;
		proxy = System::SelfEndpoint();
	}
	else proxy = endpoint;

	G2::Reader r(qka);
	while(r.Next())
	{
		switch(r.Type())
		{
			case G2::QK: qkey = r.Follow().Pod<uint32>(); break;
			case G2::SNA: 
			{
				Ip::Address addr;
				addr.value = r.Pod<uint32>();
				break;
			}
			case G2::QNA: 
			{
				remote.address.value = r.Pod<uint32>();
				if(r.BytesLeft() == 6)
					remote.port = r.Pod<uint16>(4);
			}
		}
	}

	if(qkey == 0) return;
	
	G2::HubInfo hub = System::GetHubCache()->Find(remote.address);
	if(hub.Empty())
	{
		hub.SetEndpoint(remote);
		hub.Proxy(proxy);
	}

	hub.Seen(System::Now());
	hub.QueryKey(qkey, System::Now());
	System::GetHubCache()->Update(hub);

	if(hub.Proxy().Empty())
		System::LogAdv() << "Got query key for " << hub.GetEndpoint() << " Issued for self" << std::endl;
	else 
		System::LogAdv() << "Got query key for " << hub.GetEndpoint() << " Issued for " << hub.Proxy() << std::endl;

	--needKeys_;
}

Manager::SearcherPtr Manager::SelectSearcher()
{
	if(searchers_.empty()) return SearcherPtr();

	const std::time_t now = System::Now();
	SearchersContainer::iterator imax = searchers_.begin();

	for(SearchersContainer::iterator i = searchers_.begin(); i != searchers_.end(); ++i)
	{
		if(i->second->SearchedTime() == 0) return i->second;

		if(RatedSince(now, i->second) > RatedSince(now, imax->second))
			imax = i;
	}
	return RatedSince(now, imax->second) >= BASE_RATE ? imax->second : SearcherPtr();
}

bool Manager::LocalSearch(SearcherPtr p)
{
	const std::time_t now = System::Now();

	Ip::Endpoint endpoint;
	for(NeighborsContainer::iterator i = neighbors_.begin(); i != neighbors_.end(); ++i)
	{
		const Ip::Endpoint ep = i->first;
		std::time_t& ts = i->second;

		if(now - ts > REST_TIME && p->AddSearched(ep.address))
		{
			ts = now;
			endpoint = ep; 
			break;
		}
	}

	if(endpoint.Empty()) return false;

	System::LogAdv() << "Sending search request to neighboring hub: " << endpoint << std::endl;
	G2::Packet q2;
	p->BuildQuery(q2);
	System::GetNetwork()->Send(q2, endpoint, Ip::Protocol::TCP);

	return true;
}

bool Manager::ForeignSearch(SearcherPtr p)
{
	G2::HubInfo hub = System::GetHubCache()->GetSearchableHub(NotSearched(p));
	if(hub.Empty()) return false;
	if(!hub.IsKeyValid(System::Now())) 
	{
		hub.DiscardKey();
		System::GetHubCache()->Update(hub);
		return false;
	}

	hub.SearchAfter(System::Now() + G2::FOREIGN_HUB_SEARCH_PERIOD);
	p->AddSearched(hub.GetAddress());

	System::LogAdv() << "Sending search request to " << hub.GetEndpoint() << " QKey: " << hub.QueryKey() << std::endl;

	G2::Packet q2;
	Ip::Endpoint returnEndpoint = hub.Proxy().Empty() ? System::SelfEndpoint() : hub.Proxy();
	p->BuildQuery(q2, returnEndpoint, hub.QueryKey());
	System::GetNetwork()->Send(q2, hub.GetEndpoint(), Ip::Protocol::UDP);
	System::GetHubCache()->Update(hub);
	return true;
}

void Manager::DoSearch()
{
	SearcherPtr p = SelectSearcher();
	if(!p) return;
	p->SetSearchedTime(System::Now());

	if(LocalSearch(p) || ForeignSearch(p))
		p->SetRate(p->Rate() / 2 + 1);
	else NeedMoreKeys();
}

void Manager::NeedMoreKeys()
{
	needKeys_ = MIN_QKEY_REQUESTS;
}

void Manager::OnTimer()
{
	if(!searchers_.empty() && periodSearch_.Set()) DoSearch();
	if(needKeys_ && periodQueryKey_.Set()) RequestMoreKeys();
}

void Manager::RequestMoreKeys()
{
	G2::HubInfo hub = System::GetHubCache()->GetHubWithoutKey();
	if(hub.Empty()) return;
	if(System::GetNetwork()->IsNeighbor(hub.GetEndpoint().address)) return;

	G2::Packet qkr;
	G2::Writer w(qkr);

	if(System::IsFirewalled())
	{
		const Ip::Endpoint proxy = System::GetNetwork()->RandomNeighbor();
		if(proxy.Empty()) return;

		hub.Proxy(proxy);

		w.Begin(G2::QKR);
		w.Begin(G2::QNA);
		w.Pod(hub.GetAddress().value);
		w.Pod(hub.GetPort());
		w.Close(G2::QNA);
		w.Close(G2::QKR);

		System::LogAdv() << "Requesting query key from " << hub.GetEndpoint() << 
			" Using " << hub.Proxy() << " as proxy. Want keys: " << needKeys_ << std::endl;

		System::GetNetwork()->Send(qkr, hub.Proxy(), Ip::Protocol::TCP);
	}
	else
	{
		const Ip::Endpoint self = System::SelfEndpoint();
		assert(!self.Empty());

		w.Begin(G2::QKR);
		w.Begin(G2::RNA);
		w.Pod(self.address.value);
		w.Pod(self.port);
		w.Close(G2::RNA);
		w.Close(G2::QKR);

		System::LogAdv() << "Requesting query key from " << hub.GetEndpoint() << 
			" Expecting direct reply. Want keys: " << needKeys_ << std::endl;

		System::GetNetwork()->Send(qkr, hub.GetEndpoint(), Ip::Protocol::UDP);
	}

	hub.KeyIssued(System::Now());
	System::GetHubCache()->Update(hub);
}

void Manager::HubConnected(Ip::Endpoint ep)
{
	neighbors_[ep] = 0;
}

void Manager::HubDisconnected(Ip::Endpoint ep)
{
	neighbors_.erase(ep);
	System::GetHubCache()->DiscardProxy(ep.address);
}

Manager::SearcherPtr Manager::Create(const Criteria& criteria, Manager::Type t, uint rate)
{
	GUID guid;
	guid.Generate();

	SearcherPtr p = SearcherPtr(new Searcher(criteria, rate, guid, t == Manager::USER ? idgen_.Get() : 0));
	searchers_[guid] = p;
	return p;
}

void Manager::Detach(Manager::SearcherPtr pSearcher)
{
	for(SearchersContainer::iterator i = searchers_.begin(); i != searchers_.end(); ++i)
		if(i->second == pSearcher)
		{
			searchers_.erase(i);
			break;
		}
}

Manager::SearchersContainer::iterator Manager::Find(GUID guid)
{
	SearchersContainer::iterator i = searchers_.find(guid);
	if(i == searchers_.end())
		throw Error("Bad guid");
	return i;
}

Manager::SearcherPtr Manager::Get(GUID guid)
{
	return Find(guid)->second;
}

Manager::SearcherPtr Manager::Get(ID id)
{
	for(SearchersContainer::iterator i = searchers_.begin(); i != searchers_.end(); ++i)
		if(i->second->GetID() == id)
			return i->second;
	throw Error("Bad guid");
}

} //namespace Search


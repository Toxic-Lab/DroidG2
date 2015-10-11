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

#include "searcher.hpp"
#include "g2packet.hpp"
#include <limits>
#include "system.hpp"
#include "g2misc.hpp"
#include <iostream>
#include "filesystem.hpp"
#include <boost/algorithm/string.hpp>
#include "url.hpp"

using boost::contains;
using boost::to_lower_copy;

namespace {

bool ValidateName(const std::string& name, const std::string& test)
{
	if(name.empty()) return true;

	std::string::size_type i = 0;
	int good = 0, bad = 0;

	while(i < name.size()) 
	{
		char flag;

		while(i < name.size() && name[i] == ' ') ++i;

		if(i == name.size()) break;

		if(name[i] == '-' || name[i] == '+') 
		{
			flag = name[i];
			if(++i == name.size()) break;
		} 
		else flag = '+';

		int j = i;
		if(name[i] == '"') 
		{
			if(++i == name.size()) break;
			while(i < name.size() && name[i] != '"') ++i;
		}
		else
			while(i < name.size() && name[i] != ' ') ++i;

		const bool found = test.find(name.substr(j, i - j)) != std::string::npos;
		if(found && flag == '+') ++good;
		else if(found || flag == '+') ++bad;

		if(i < name.size() && name[i] == '"') --i; //closing "
	}

	return 2 * good > bad;
}

bool ValidateHit(const Search::Hit& hit, const Search::Criteria& criteria)
{
	const file_offset_t min = criteria.min;
	const file_offset_t max = criteria.max;

	const std::string dn = Url::Decode(hit.dn);
	if(dn.find('\\') != std::string::npos) return false; 
	if(dn.find('/') != std::string::npos) return false; 
	if(dn.find('\0') != std::string::npos) return false; 
	if(dn == "." || dn == "..") return false;

	if(criteria.media)
	{
		if(hit.dn.empty()) return false;
		if(Media::ResolveByName(hit.dn) != criteria.media) return false;
	}

	if(!criteria.extensions.empty())
	{
		std::string ext = FileSystem::Extension(hit.dn);
		boost::to_lower(ext);
		if(!ext.empty() && ext[0] == '.') ext.erase(0, 1);
		if(criteria.extensions.find(ext) == criteria.extensions.end())
			return false;
	}

	if(hit.sha1.Empty() && hit.ttr.Empty() && 
		hit.md5.Empty() && hit.ed2k.Empty()) 
		return false;
				
	if(min && hit.size < min) return false;
	if(max && hit.size > max) return false;
				
	if( !(criteria.sha1.Empty() && criteria.ttr.Empty() && 
		criteria.md5.Empty() && criteria.ed2k.Empty()) )
	{
		const bool match = 
		(!criteria.sha1.Empty() && criteria.sha1 == hit.sha1) ||
		(!criteria.ttr.Empty() && criteria.ttr == hit.ttr) ||
		(!criteria.md5.Empty() && criteria.md5 == hit.md5) ||
		(!criteria.ed2k.Empty() && criteria.ed2k == hit.ed2k);

		if(!match) return false;;
	}

	if(!ValidateName(to_lower_copy(criteria.dn), to_lower_copy(hit.dn))) 
		return false;

	return true;
}

} //namespace 

namespace Search {

template <typename T> 
void InsertHash(G2::Writer& w, const T& hash, const std::string& prefix)
{
	w.Begin(G2::URN);
	w.Write(prefix.c_str(), prefix.size());
	w.Pod(char(0));
	w.Write(hash.Bytes(), hash.Size());
	w.Close(G2::URN);
}

Searcher::Searcher(const Criteria& criteria, uint rate, GUID guid, ID id):
	criteria_(criteria),
	rate_(rate),
	guid_(guid),
	id_(id),
	timestamp_(0)
{
}

void Searcher::SetRate(uint rate)
{
	rate_ = rate;
}

Criteria Searcher::GetCriteria() const
{
	return criteria_;
}

Result Searcher::GetResult(ID id) const
{
	std::size_t i = 0;
	for(i = 0; i < results_.size(); ++i)
		if(results_[i].id == id)
			break;

	if(i == results_.size()) throw std::runtime_error("Bad id");

	return results_[i];
}	

ID Searcher::GetID()
{
	return id_;
}


void TranslateHit(G2::Reader r, Hit& hit)
{
	while(r.Next())
	{
		switch(r.Type())
		{
		case G2::URN:
			G2::ExtractUrn(r.Follow(), hit.sha1, hit.ttr, hit.md5, hit.ed2k);
			break;
		case G2::DN:
		{
			int offset = 0;
			if(!r.Contains(G2::SZ)) 
			{
				hit.size = r.Pod<uint32>();
				offset = 4;
			}
			hit.dn.resize(r.BytesLeft() - offset);
			r.Read(&hit.dn[0], hit.dn.size(), offset);
			break;
		}
		case G2::SZ:
			hit.size = r.BytesLeft() == 8 ? r.Pod<int64>() : r.Pod<int32>();
			break;
		}
	}

	/*
	assert(h.type == G2::PacketType::H);

	for(G2::PacketsContainer::const_iterator i = h.packets.begin(); i != h.packets.end(); ++i)
	{
		switch (i->type) 
		{
		case G2::PacketType::URN:
			ExtractUrn(*i, hit.sha1, hit.ttr, hit.md5, hit.ed2k);
			break;
		case G2::PacketType::DN: 
		{
			int offset = 0;
			if(!i->packets.contains(G2::PacketType::SZ)) 
			{
				hit.size = i->payload.read_pod<uint32>();
				offset = 4;
			}

			i->payload.read(std::back_inserter(hit.dn), i->payload.size() - offset, offset);
			break;
		}

		case G2::PacketType::SZ: 
			hit.size = i->payload.size() == 8 ? 
				i->payload.read_pod<int64>() : i->payload.read_pod<int32>();
			break;
			//TODO other children
		}
	}
	*/
}

bool Searcher::Searched(Ip::Address addr)
{
	HubsContainer::const_iterator i = searchedHubs_.find(addr);
	return i != searchedHubs_.end() && System::Now() - i->second < 60 * 60;
}

bool Searcher::AddSearched(Ip::Address addr)
{
	const bool success = !Searched(addr);

	if(success) searchedHubs_[addr] = System::Now();

	return success;
}

//void Searcher::BuildQuery(G2::Packet& q2)
//{

	/*
	q2.type = G2::PacketType::Q2;
	q2.payload.write(guid_.Get(), guid_.Get() + GUID::Size());
	if(criteria_.min || criteria_.max)
	{
		G2::PacketIterator i = q2.packets.add(G2::PacketType::SZR);
		i->payload.write_pod(criteria_.min);
		i->payload.write_pod(criteria_.max == 0 ? std::numeric_limits<uint64>::max() : criteria_.max);
	}

	if(!criteria_.dn.empty())
		q2.packets.add(G2::Packet(G2::PacketType::DN, G2::Payload(criteria_.dn.begin(), criteria_.dn.end())));

	if(!criteria_.sha1.Empty() && !criteria_.ttr.Empty())
		InsertHash(q2, Hashes::BP(criteria_.sha1, criteria_.ttr), "bp");
	else
	{
		if(!criteria_.sha1.Empty())
   			InsertHash(q2, criteria_.sha1, "sha1");
		if(!criteria_.ttr.Empty())
   			InsertHash(q2, criteria_.ttr, "ttr");
	}

	if(!criteria_.md5.Empty())
		InsertHash(q2, criteria_.md5, "md5");

	if(!criteria_.ed2k.Empty())
		InsertHash(q2, criteria_.ed2k, "ed2k");
		*/
//}

void Searcher::BuildQuery(G2::Packet& q2)
{
	BuildQuery(q2, Ip::Endpoint(), 0);
}

void Searcher::BuildQuery(G2::Packet& q2, Ip::Endpoint endpoint, uint32 qkey)
{
	G2::Writer w(q2);

	w.Begin(G2::Q2);

	if(!endpoint.Empty())
	{
		w.Begin(G2::UDP);
		w.Pod(endpoint.address.value);
		w.Pod(endpoint.port);
		w.Pod(qkey);
		w.Close(G2::UDP);
	}

	if(criteria_.min || criteria_.max)
	{
		w.Begin(G2::SZR);
		w.Pod(criteria_.min);
		w.Pod(criteria_.max == 0 ? std::numeric_limits<uint64>::max() : criteria_.max);
		w.Close(G2::SZR);
	}

	if(!criteria_.dn.empty())
	{
		w.Begin(G2::DN);
		w.Write(&criteria_.dn[0], criteria_.dn.size());
		w.Close(G2::DN);
	}

	if(!criteria_.sha1.Empty() && !criteria_.ttr.Empty())
		InsertHash(w, Hashes::BP(criteria_.sha1, criteria_.ttr), "bp");
	else
	{
		if(!criteria_.sha1.Empty())
   			InsertHash(w, criteria_.sha1, "sha1");
		if(!criteria_.ttr.Empty())
   			InsertHash(w, criteria_.ttr, "ttr");
	}

	if(!criteria_.md5.Empty())
		InsertHash(w, criteria_.md5, "md5");

	if(!criteria_.ed2k.Empty())
		InsertHash(w, criteria_.ed2k, "ed2k");

	w.Write(guid_.Get(), GUID::Size());
	w.Close(G2::Q2);

	/*
	BuildQuery(q2);
	G2::PacketIterator i = q2.packets.add(G2::PacketType::UDP);
	i->payload.write_pod(endpoint.address.value);
	i->payload.write_pod(endpoint.port);
	i->payload.write_pod(qkey);
	*/
}

/*
void fun(const G2::Packet& pk)
{
	for(int j = 0; j < 1000; ++j)
	{
		std::cout << j << std::endl;
		for(int i = 0; i < 1000; ++i)
		{
			std::vector<char> pack;
			pk.Pack(pack);
	//		G2::Packet pk2;
	//		pk2.Assign(&pack[0], pack.size());
		}
	}
}
*/

void Searcher::OnHit(const G2::Packet& qh2, Ip::Endpoint fromEndpoint)
{
//	fun(qh2);
	if(rate_ == 0) return;

	std::vector <Hit> hits;
	std::vector <Ip::Endpoint> hubs;
	Ip::Endpoint senderEndpoint = fromEndpoint;
	GUID senderGuid;
	std::string nick;
	G2::Reader r(qh2);
	while(r.Next())
	{
		switch(r.Type())
		{
			case G2::GU:
			{
				r.Read(senderGuid.Bytes(), 16);
				if(searchedGuids_.find(senderGuid) != searchedGuids_.end())
					return;
				if(senderGuid == System::Guid()) return;
				break;
			}
			case G2::NA:
				senderEndpoint = Ip::Endpoint(r.Pod<uint32>(), r.Pod<uint16>(4));
				break;
			case G2::H:
			{
				Hit hit;
				TranslateHit(r.Follow(), hit);
				if(ValidateHit(hit, criteria_))
					hits.push_back(hit);				
				break;
			}

			case G2::NH:
				hubs.push_back(Ip::Endpoint(r.Pod<uint32>(), r.Pod<uint16>(4)));
				break;
			case G2::UPRO:
			{
				G2::Reader rUpro = r.Follow();
				while(rUpro.Next())
				{
					if(rUpro.Type() == G2::NICK)
						nick = std::string(rUpro.Data(), rUpro.BytesLeft());
				}
			break;
			}



		}
	}
		/*
	for(G2::PacketsContainer::const_iterator i = qh2.packets.begin(); i != qh2.packets.end(); ++i)
	{
		switch (i->type)
		{
			case G2::PacketType::GU: 
				i->payload.read(senderGuid.Bytes(), 16); 
				if(searchedGuids_.find(senderGuid) != searchedGuids_.end())
					return;
				break;

			case G2::PacketType::NA:
				senderEndpoint = 
					Ip::Endpoint(i->payload.read_pod<uint32>(), 
						i->payload.read_pod<uint16>(4));
				break;

			case G2::PacketType::V: break; //TODO it

			case G2::PacketType::H: 
			{
				QueryHit hit;
				TranslateHit(*i, hit);
				if(ValidateHit(hit, criteria_))
					hits.push_back(hit);				
				break;
			}

			case G2::PacketType::NH: 
				hubs.push_back(Ip::Endpoint(i->payload.read_pod<uint32>(), 
							i->payload.read_pod<uint16>(4))); 
				break;
		}

	}
	*/

	//searchedNodes_.insert(senderEndpoint);
	searchedGuids_.insert(senderGuid);

	for(std::size_t i = 0; i < hits.size(); i++) 
	{
		Result sr;
		sr.id = results_.size();
		sr.host.guid = senderGuid;
		sr.host.nick = nick;
		sr.host.endpoint = senderEndpoint;
		sr.host.hubs = hubs;
		sr.hit = hits[i];
		results_.push_back(sr);
	}
}

} //namespace Search

/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010  Andrey Stroganov <savthe@gmail.com>

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

#include <iterator>

#include <boost/bind/apply.hpp>

#include "g2net.hpp"
#include "foreach.hpp"
#include "gatekeeper.hpp"
#include "g2handshake.hpp"
#include "discovery.hpp"
#include "settings.hpp"
#include "g2node.hpp"
#include "g2packet.hpp"
#include "identity.hpp"
#include "transceiver.hpp"
#include "system.hpp"
#include "hubcache.hpp"
#include "sharemanager.hpp"
#include "g2misc.hpp"
#include "searchdefs.hpp"
#include "searchmanager.hpp"
#include <android/log.h>

namespace {

const std::time_t FIREWALL_CHECK_PERIOD = 20;

void AddUrn( G2::Writer& w, const std::string& prefix, const char* p, uint size)
{
	w.Begin( G2::URN );
	w.Write( prefix );
	w.Pod( char(0) );
	w.Write( p, size );
	w.Close( G2::URN );
}

}


G2Net::G2Net(ba::io_service& io):
	acceptor_(io, ba::ip::tcp::v4()), 
	pTransceiver_(new Transceiver(io)),
	extcount_(0),
	tsFirewall_(0),
	periodNodesTimer_(Time::Seconds(5)),
	periodConnect_(Time::Milliseconds(250))
{
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Init. gunutella 2 net component.");
}

void G2Net::AddHandler(G2::PacketType packetType, PacketHandlerFunction handler)
{
	handlers_[packetType].push_back(handler);
}

void G2Net::Shutdown()
{
	for(Nodes::iterator i = hubs_.begin(); i != hubs_.end(); ++i)
		i->second->Close();
	hubs_.clear();

	for(Handshakes::iterator i = handshakes_.begin(); i != handshakes_.end(); ++i)
		i->second->Close();
	handshakes_.clear();

	foreach(GatekeeperPtr p, gatekeepers_) p->Close();
	gatekeepers_.clear();
}

bool G2Net::IsNeighbor(Ip::Address addr) const
{
	return hubs_.find(addr) != hubs_.end();
}

void G2Net::Start()
{
	const uint port = System::GetSettings()->net.listenPort;
	endpoint_.port = port;

	ba::socket_base::reuse_address reuse(true);
	if(System::GetSettings()->net.directAble != Settings::NO)
	{
		try
		{
			acceptor_.set_option(reuse);
			acceptor_.bind(ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port));
			acceptor_.listen();

			Gatekeeper::Ptr pGatekeeper = GatekeeperPtr(new Gatekeeper);
			acceptor_.async_accept(pGatekeeper->Socket(), boost::bind(&G2Net::OnAccept, this, pGatekeeper, ba::placeholders::error));
			System::LogBas() << "Accepting connections on port: " << port << std::endl;
		}
		catch(std::exception& e)
		{
			System::LogBas() << "Couldn't bind to port " << port << ". Check if no other application is using it." << std::endl;
			throw;
		}
	}
	else System::LogBas() << "Warning! Incomming connections are restricted. Sharelin will be less effective. See sharelin.conf" << std::endl;

	pTransceiver_->Start(port);

	AddHandler(G2::PUSH, boost::bind(&G2Net::OnPacketPUSH, this, _1, _2, _3));
}

void G2Net::OnPacketPUSH(const G2::Packet& pk, Ip::Endpoint endpoint, Ip::Protocol::Type proto)
{
	System::LogAdv() << "Got push from " << endpoint << std::endl;
	G2::Reader r(pk);
	GUID guid;
	while(r.Next())
	{
		if(r.Type() == G2::TO)
			r.Read(guid.Bytes(), 16);
	}

	if(guid != System::Guid())
	{
		System::LogAdv() << "Pushed wrong guid from " << endpoint << std::endl;
		return;
	}

	Ip::Endpoint e;
	e.address.value = r.Pod<uint32>();
	e.port = r.Pod<uint16>();

	GatekeeperPtr p(new Gatekeeper);
	gatekeepers_.insert(p);
	p->Push(e);
}

G2Net::~G2Net()
{
	delete pTransceiver_;
	pTransceiver_ = 0;
}

void G2Net::Detach(NodePtr p)
{
	Ip::Address addr = p->Info().endpoint.address;
	System::LogAdv() << "Terminating connection to hub " << addr << std::endl;
	hubs_.erase(addr);
}

void G2Net::Detach(HandshakePtr p)
{
	Ip::Address addr = p->GetEndpoint().address;
	System::LogAdv() << "Terminating handshake " << addr << std::endl;
	if(!p->Alive()) System::GetHubCache()->Remove(addr);
	handshakes_.erase(addr);
}

void G2Net::Detach(GatekeeperPtr p)
{
	gatekeepers_.erase(p);
}

Ip::Endpoint G2Net::RandomNeighbor() const
{
	if(hubs_.empty()) return Ip::Endpoint();

	Nodes::const_iterator i = hubs_.begin();
	std::advance(i, rand() % hubs_.size());

	return i->second->Info().endpoint;
}

void G2Net::CheckFirewall(Ip::Address addr)
{
	System::LogAdv() << "Checking if we are firewalled via " << addr << std::endl;
	G2::Packet pi;
	G2::Writer w(pi);

	w.Begin(G2::PI);
	w.Begin(G2::UDP);
	w.Pod(endpoint_.address.value);
	w.Pod(endpoint_.port);
	w.Close(G2::UDP);
	w.Close(G2::PI);

	if(extcount_ > 0) --extcount_;

	Send(pi, addr, Ip::Protocol::TCP);

	tsFirewall_ = System::Now();
}

void G2Net::OnTimer()
{
	const bool mayConnect = 
		hubs_.size() < System::GetSettings()->net.maxHubs &&
		handshakes_.size() < 15 &&
		periodConnect_.Set();

	if(mayConnect) ConnectRandomHub();

	if(!hubs_.empty() && System::Now() - tsFirewall_ > FIREWALL_CHECK_PERIOD)
		CheckFirewall(RandomNeighbor().address);

	if(periodNodesTimer_.Set())
	{
		Nodes tmp = hubs_;
		for(Nodes::iterator i = tmp.begin(); i != tmp.end(); ++i)
			i->second->OnTimer();
	}
}

void G2Net::Dispatch(const G2::Packet& packet)
{
	for(Nodes::iterator i = hubs_.begin(); i != hubs_.end(); ++i)
		i->second->Send(packet);
}

void G2Net::OnPacket(const G2::Packet& packet, Ip::Endpoint ep, Ip::Protocol::Type proto)
{
	if(proto == Ip::Protocol::TCP)
		System::GetHubCache()->Touch(ep.address, System::Now());
	G2::Reader r(packet);
	//std::cout << "GOT " << (proto == Ip::Protocol::TCP ? "TCP" : "UDP" ) << " PACKET: " << StaticName::ToString(r.Type()) << std::endl;
	DefaultPacketHandler(packet, ep, proto);
	Handlers::iterator i = handlers_.find(r.Type());
	if(i != handlers_.end())
		std::for_each(i->second.begin(), i->second.end(), 
				boost::bind(boost::apply<void>(), _1, packet, ep, proto));
}

void G2Net::DefaultPacketHandler(const G2::Packet& packet, Ip::Endpoint ep, Ip::Protocol::Type proto)
{
	G2::Reader r(packet);
	switch(r.Type())
	{
		case G2::PO:
		{
			if(r.Contains(G2::RELAY) && proto == Ip::Protocol::UDP && extcount_ < 100) 
			{
				if(extcount_ == 0) 
					System::LogAdv() << "Yoopy! I'm not firewalled." << std::endl;
				extcount_ = 100;
			}
			break;
		}
		case G2::Q2:
		{
			OnPacketQ2(packet, ep, proto);
			break;
		}

	}
}

void G2Net::OnPacketQ2(const G2::Packet& packet, Ip::Endpoint ep, Ip::Protocol::Type proto)
{
	Search::Criteria criteria;
	Ip::Endpoint returnEndpoint;
	GUID guid;
	std::string dn;

	G2::Reader r(packet);
	bool searchUrn = false;

	while( r.Next() )
	{
		switch ( r.Type() )
		{
			case G2::DN: 
			{
				dn.resize(r.BytesLeft());
				r.Read(&dn[0], dn.size());
				break;
			}
			case G2::UDP: returnEndpoint = Ip::Endpoint(r.Pod<uint32>(), r.Pod<uint16>(4)); break;
			case G2::URN:
			{
				searchUrn = true;
				G2::ExtractUrn(r, criteria.sha1, criteria.ttr, criteria.md5, criteria.ed2k);
				break;
			}
		}
	}

	r.Read(guid.Bytes(), 16);

	std::vector< Share::ManagedInfo > results;

	if( searchUrn )
	{
		//std::cout << "." << std::flush;
		try 
		{ 
			Share::ManagedInfo inf = System::GetShareMgr()->GetByHashes(criteria.sha1, criteria.ttr, criteria.ed2k, criteria.md5); 
			inf.IncreaseHits();
			results.push_back(inf);
			System::LogAdv() << "Successful hit: " << inf.Name() << std::endl;
		}
		catch (Share::NotFound&) {}
	}
	else if(!dn.empty())
		System::GetShareMgr()->FindByDn(dn, std::back_inserter(results));


	if( results.empty() ) return;
	
	G2::Packet qh;
	G2::Writer w(qh);

	w.Begin( G2::QH2 );
		w.Begin( G2::NA );
			w.Pod(endpoint_.address.value);
			w.Pod(endpoint_.port);
			//std::cout << endpoint << std::endl;
		w.Close( G2::NA );

		w.Begin( G2::GU );
			w.Write( System::Guid().Bytes(), 16 );
		w.Close( G2::GU );

		for(Nodes::iterator i = hubs_.begin(); i != hubs_.end(); ++i)
		{
			const Ip::Endpoint ep = i->second->Info().endpoint;
			w.Begin(G2::NH);
				w.Pod(ep.address.value);
				w.Pod(ep.port);
			w.Close(G2::NH);
		}

		w.Begin( G2::UPRO );
			w.Begin( G2::NICK );
				w.Write( System::GetSettings()->general.userName );
			w.Close( G2::NICK );
		w.Close( G2::UPRO );	

		for(uint i = 0; i < results.size(); ++i)
		{
			const Share::ManagedInfo& inf = results[i];

			w.Begin( G2::H );
			if( !inf.Sha1().Empty() && !inf.Ttr().Empty() )
			{
				Hashes::BP bp( inf.Sha1(), inf.Ttr() );
				AddUrn( w, "bp", bp.Bytes(), Hashes::BP::Size() );
			}
			else if( !inf.Sha1().Empty() )
				AddUrn( w, "sha1", inf.Sha1().Bytes(), Hashes::SHA1::Size() );
			else if( !inf.Ttr().Empty() )
				AddUrn( w, "ttr", inf.Ttr().Bytes(), Hashes::TTR::Size() );

			if( !inf.Ed2k().Empty() )
				AddUrn( w, "ed2k", inf.Ed2k().Bytes(), Hashes::ED2K::Size() );
			if( !inf.Md5().Empty() )
				AddUrn( w, "md5", inf.Md5().Bytes(), Hashes::MD5::Size() );

			w.Begin( G2::URL );
			w.Close( G2::URL );

			w.Begin( G2::SZ );
				w.Pod( inf.Size() );
			w.Close( G2::SZ );

			w.Begin( G2::DN );
				w.Write( inf.Name() );
			w.Close( G2::DN );

			w.Close( G2::H );
		}
				
		w.Pod( char(0) );
		w.Write( guid.Bytes(), 16 );
	w.Close( G2::QH2 );

	if( returnEndpoint.Empty() ) Send( qh, ep, Ip::Protocol::TCP );
	else Send( qh, returnEndpoint, Ip::Protocol::UDP );
}

void G2Net::Send(const G2::Packet& packet, Ip::Endpoint endpoint, Ip::Protocol::Type proto)
{
	if(proto == Ip::Protocol::UDP)
	{
		pTransceiver_->Send(packet, endpoint);
	}
	else if(hubs_.find(endpoint.address) != hubs_.end())
	{
		hubs_[endpoint.address]->Send(packet);
	}
}

void G2Net::ConnectRandomHub()
{
	uint tries = 10;
	while(tries --> 0)
	{
		const G2::HubInfo hub = System::GetHubCache()->GetRandomHub();
		if(hub.Empty()) break;

		const Ip::Endpoint endpoint = hub.GetEndpoint();

		const bool connected = hubs_.find(endpoint.address) != hubs_.end() || 
			handshakes_.find(endpoint.address) != handshakes_.end();

		if(!connected)
		{
			HandshakePtr p(new G2Handshake);
			handshakes_[endpoint.address] = p;
			p->Connect(endpoint);
			break;
		}
	}
}

void G2Net::OnHubConnected(G2Handshake* pHandshake)
{
	System::LogBas() << "Connected to G2 hub " << pHandshake->GetEndpoint() << std::endl;

	const Ip::Endpoint endpoint = pHandshake->GetEndpoint();
	const Ip::Address address = endpoint.address;
	
	assert(hubs_.find(address) == hubs_.end());
	assert(handshakes_.find(address) != handshakes_.end());

	NodePtr p(new G2Node(pHandshake));
	hubs_[address] = p;
	p->Start();

	handshakes_.erase(address);

	if(hubs_.size() == System::GetSettings()->net.maxHubs)
	{
		for(Handshakes::iterator i = handshakes_.begin(); i != handshakes_.end(); ++i)
			i->second->Close();
		handshakes_.clear();
	}

	const Ip::Address self = pHandshake->SelfAddress();
	if(!self.Empty())
	{
		System::LogAdv() << "Resolved self IP: " << self << std::endl;
		endpoint_.address = self;
	}

	CheckFirewall(address);

	System::GetSearchMgr()->HubConnected(endpoint);
}

bool G2Net::IsFirewalled() const
{
	if(endpoint_.Empty()) return true;
	const Settings* p = System::GetSettings();

	return !(p->net.directAble == Settings::YES || 
		(p->net.directAble == Settings::AUTO && extcount_));
}

void G2Net::OnAccept(Gatekeeper::Ptr p, const bs::error_code& error)
{
	gatekeepers_.insert(p);
	p->Start();
	Gatekeeper::Ptr pGatekeeper = Gatekeeper::Ptr(new Gatekeeper);
	acceptor_.async_accept(pGatekeeper->Socket(), boost::bind(&G2Net::OnAccept, this, pGatekeeper, ba::placeholders::error));
}


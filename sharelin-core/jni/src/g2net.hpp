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

#ifndef G2NET_HPP
#define G2NET_HPP

#include <stdexcept>
#include <set>
#include <ctime>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "types.hpp"
#include "ip.hpp"
#include "period.hpp"
#include "io.hpp"
#include "g2packet/packettype.hpp"
#include "netfwd.hpp"

class Transceiver;

namespace G2 { class Packet; }

class G2Net
{
	typedef boost::function3<void, const G2::Packet&, Ip::Endpoint, Ip::Protocol::Type> PacketHandlerFunction;

public:
	G2Net(ba::io_service&);
	~G2Net();

	void Start();
	void OnHubConnected(G2Handshake*);
	void Detach(HandshakePtr);
	void Detach(NodePtr);
	void Detach(GatekeeperPtr);
	void OnPacket(const G2::Packet& packet, Ip::Endpoint, Ip::Protocol::Type proto);
	void Dispatch(const G2::Packet& packet);
	void Send(const G2::Packet& packet, Ip::Endpoint, Ip::Protocol::Type proto);
	void OnTimer();
	void Shutdown();
	Ip::Endpoint RandomNeighbor() const;
	Ip::Endpoint SelfEndpoint() const { return endpoint_; }
	bool IsNeighbor(Ip::Address) const;
	bool IsFirewalled() const;
	void AddHandler(G2::PacketType packetType, PacketHandlerFunction handler);

	template <typename T>
	void CopyHubs(T iOut) const
	{
		for(Nodes::const_iterator i = hubs_.begin(); i != hubs_.end(); ++i)
			*iOut++ = i->second;
	}

	template <typename T>
	void CopyHandshakes(T iOut) const
	{
		for(Handshakes::const_iterator i = handshakes_.begin(); i != handshakes_.end(); ++i)
			*iOut++ = i->second;

	}

private:
	typedef std::map<Ip::Address, NodePtr> Nodes;
	typedef std::map<Ip::Address, HandshakePtr> Handshakes;
	typedef std::set<GatekeeperPtr> Gatekeepers;
	typedef std::list <PacketHandlerFunction> HandlersContainer;
	typedef std::map<G2::PacketType, HandlersContainer> Handlers;

	void DefaultPacketHandler(const G2::Packet& packet, Ip::Endpoint, Ip::Protocol::Type proto);
	void OnPacketQ2(const G2::Packet& packet, Ip::Endpoint, Ip::Protocol::Type proto);
	void OnPacketPUSH(const G2::Packet& packet, Ip::Endpoint, Ip::Protocol::Type proto);
	void OnAccept(GatekeeperPtr, const bs::error_code& error);
	void ConnectRandomHub();
	void PingHost(Ip::Endpoint);
	void CheckFirewall(Ip::Address);

	Nodes hubs_;
	Handshakes handshakes_;
	Gatekeepers gatekeepers_;
	ba::ip::tcp::acceptor acceptor_;
	ba::streambuf request_;
	ba::streambuf response_;
	Transceiver* pTransceiver_;
	Handlers handlers_;
	unsigned int extcount_;
	Ip::Endpoint endpoint_;
	std::time_t tsFirewall_;
	Time::Period periodNodesTimer_;
	Time::Period periodConnect_;
};

#endif //G2NET_HPP

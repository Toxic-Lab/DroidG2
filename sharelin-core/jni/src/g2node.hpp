/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010  Andrey Stroganov

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

#ifndef G2NODE_HPP
#define G2NODE_HPP

#include <vector>
#include <queue>
#include <ctime>

#include <boost/enable_shared_from_this.hpp>

#include "io.hpp"
#include "types.hpp"
#include "period.hpp"
#include "ip.hpp"
#include "period.hpp"
#include "g2packet.hpp"
#include "g2nodeinfo.hpp"

class G2Handshake;

class G2Node: public boost::enable_shared_from_this<G2Node>
{
public:
	G2Node(G2Handshake*);
	~G2Node();
	G2::NodeInfo Info();
	void Close();
	void Send(const G2::Packet& pk);
	void OnTimer();
	void Start();

private:
	void UpdateLNI();
	void DetachMe();
	void OnControlByte(const bs::error_code& err);
	void OnPacketHeader(const bs::error_code& err);
	void OnPayload(const bs::error_code& err);
	void HandlePacket();
	void ReadPacket();
	void ExtractHubs(const std::string& strHubs);
	void OnPacketSent(const bs::error_code& err);
	void SendPackets();
	void UpdateQHT();
	void ResetQHT();
	void SendQHT();
	void OnPacketPO(const G2::Packet&);
	void Ping();

	TcpSocketPtr pSocket_;
	std::vector<char> g2buffer_;
	std::vector<char> rawPacket_;
	std::time_t timestamp_;
	std::time_t startTime_;
	Time::Period periodLNI_;
	Time::Period periodPing_;
	G2::NodeInfo myInfo_;
	std::queue<G2::Packet> dispatchQueue_;
	std::vector<byte> packetBuf_;
	Time::Period periodQHT_;
	std::time_t qhtTime_;
	bool closed_;
};

#endif //G2NODE_HPP

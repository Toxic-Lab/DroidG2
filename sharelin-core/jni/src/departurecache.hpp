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

#ifndef DEPARTURECACHE_HPP
#define DEPARTURECACHE_HPP

#include <cstdlib>
#include <list>
#include <ctime>

#include "udp.hpp"
#include "types.hpp"
#include "ip.hpp"
#include "bytevector.hpp"

namespace G2{ class Packet; }

class DepartureCache
{
public:
	typedef std::vector<char> Payload;
	void Acknowlege(udp::PacketID id, uint8 number);
	void Put(const G2::Packet& packet, Ip::Endpoint ep, bool acknowledge = false);
	void Put(udp::Header header, Ip::Endpoint ep);
	uint Get(char* buffer, Ip::Endpoint& endpoint);
		
private:
	struct Packet
	{
		Packet(): timestamp(0), attempts(0) {}
		Packet(udp::Header h, Ip::Endpoint e): header(h), timestamp(0), attempts(0), endpoint(e) {}
		udp::Header header;
		std::time_t timestamp;
		uint8 attempts;
		Payload payload;
		Ip::Endpoint endpoint;
	};

	typedef std::list<Packet> Cache;
	enum {DELAY = 5};
	enum {MAX_ATTEMPTS = 3};
	enum {MTU = 512};
	enum {PAYLOAD_SIZE = MTU - udp::HEADER_SIZE};
	enum {MIN_DEFLATE = PAYLOAD_SIZE + 1};
	Cache cache_;
};

#endif //DEPARTURECACHE_HPP


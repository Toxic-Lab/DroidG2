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

#include <cassert>

#include "departurecache.hpp"
#include "buffer.hpp"
#include "g2packet.hpp"

void DepartureCache::Acknowlege(udp::PacketID id, uint8 number)
{
	for(Cache::iterator i = cache_.begin(); i != cache_.end(); ++i)
	{
		if(i->header.id == id && i->header.partNumber == number)
		{
			cache_.erase(i);
			return;
		}
	}
}
	
uint DepartureCache::Get(char* buffer, Ip::Endpoint& endpoint)
{
	uint size = 0;
	if(!cache_.empty() && cache_.begin()->timestamp + DELAY < time(0))
	{
		const Cache::iterator i = cache_.begin();
		size = udp::HEADER_SIZE + i->payload.size();
		assert(udp::MTU >= size);
		i->header.Pack(buffer);
		std::memcpy(buffer + udp::HEADER_SIZE, i->payload.data(), i->payload.size());
		endpoint = i->endpoint;

		i->attempts++;
		i->timestamp = time(0);

		if(i->header.acknowledge && i->attempts < MAX_ATTEMPTS)
		cache_.push_back(*i);

		cache_.erase(i);
	}
	return size;
}

void DepartureCache::Put(udp::Header header, Ip::Endpoint endpoint)
{
	cache_.push_back(Packet(header, endpoint));
}

void DepartureCache::Put(const G2::Packet& packet, Ip::Endpoint endpoint, bool acknowledge)
{
	Buffer buf;
	buf.assign(packet.Bytes(), packet.Bytes() + packet.Size());
	//std::cout << "Departure: " << StaticName::ToString(packet.type) << std::endl;

	udp::Header hd;
	hd.deflate = false;
	if(buf.size() > MIN_DEFLATE)
	{
		try
		{
			buf.compress();
		}
		catch(ZLib::Error& err)
		{
			std::cerr << "UDP packet compress error: " << err.what() << std::endl;
			return;
		}
		hd.deflate = true;
	}
	hd.acknowledge = acknowledge;
	hd.id = rand() & 0xFFFF;
	hd.totalParts = buf.size() / PAYLOAD_SIZE + (buf.size() % PAYLOAD_SIZE ? 1 : 0);
	hd.partNumber = 0;
	
	for(uint i = 0; i < buf.size(); i += PAYLOAD_SIZE)
	{
		Packet pk;
		pk.header = hd;
		pk.header.partNumber++;
		pk.endpoint = endpoint;
		pk.payload.assign(buf.begin() + i, 
				buf.begin() + (buf.size() - i > PAYLOAD_SIZE ? PAYLOAD_SIZE : buf.size() - i));

		cache_.push_back(pk);
	}
}

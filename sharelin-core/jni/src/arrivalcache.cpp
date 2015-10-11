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

#include <ctime>
#include "zlib.hpp"
#include "foreach.hpp"
#include "g2packet.hpp"
#include "arrivalcache.hpp"
#include <cassert>

bool ArrivalCache::Put(const char* buffer, uint length, Ip::Endpoint endpoint, G2::Packet& pk)
{
	udp::Header header;
	if(!header.Unpack(buffer, length)) 
		return false;

	buffer += 8;
	length -= 8;

	if(header.partNumber == 0)
	{
		std::cout << "FIXME: partNumber = 0" << std::endl;
		return false;
	}

	Cache::iterator itPacket = cache_.find(header.id);

	if(itPacket == cache_.end())
	{
		itPacket = cache_.insert(std::make_pair(header.id, Packet(endpoint, header))).first; 
		for(uint i = 0; i < itPacket->second.parts.size(); ++i)
		{
			itPacket->second.parts[i] = pool_.Get();
			assert( itPacket->second.parts[i]);
		}
	}

	Packet& packet = itPacket->second;
	if(header.totalParts != packet.parts.size())
	{
		return false;
	}

	if(packet.parts.at(header.partNumber - 1)->empty() && length > 0)
	{
		packet.parts.at(header.partNumber - 1)->assign(buffer, length);
		packet.validParts++;

		if(packet.validParts == packet.parts.size())
		{
			bool good = OnCompleted(packet, pk);
			ErasePacket(itPacket);
			return good;
		}
	}
	/*
	else if(!i->second.parts[header.partNumber - 1].empty())
	{
		//std::cout << "CHUNK is avaliable" << std::endl;
	}
	*/
	return false;
}

bool ArrivalCache::OnCompleted(const ArrivalCache::Packet& pk, G2::Packet& g2pk)
{
	std::size_t size = 0;
	foreach(ByteVector* p, pk.parts) size += p->size();

	zbuf_.clear();
	foreach(ByteVector* p, pk.parts) zbuf_.append(p->data(), p->size());

	if(pk.header.deflate)
	{
		try
		{
			G2::Packet::Buffer* p = g2pk.GetBuffer();
			p->resize(size * 10);
			uint actualSize = ZLib::Uncompress(zbuf_.data(), zbuf_.size(), p->data(), p->size());
			p->resize(actualSize);
		}
		catch(ZLib::Error& e)
		{
			return false;
		}
	}
	else g2pk.GetBuffer()->assign(zbuf_.data(), zbuf_.size());

	return true;
}

void ArrivalCache::PurgeOld()
{
	std::time_t now = std::time(0);

	for(Cache::iterator i = cache_.begin(); i != cache_.end();)
	{
		if(now - i->second.created > 10)
		{
			Cache::iterator next = i;
			++next;
			ErasePacket(i);
			i = next;
		}
		else ++i;
	}
}

void ArrivalCache::ErasePacket(ArrivalCache::Cache::iterator itPacket)
{
	for(uint i = 0; i < itPacket->second.parts.size(); ++i)
		pool_.Release(itPacket->second.parts[i]);
	cache_.erase(itPacket);
}


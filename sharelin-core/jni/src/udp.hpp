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

#ifndef UDP_HPP
#define UDP_HPP

#include "types.hpp"

namespace udp {

const uint MTU = 512;
const uint HEADER_SIZE = 8;

typedef uint16 PacketID;

struct Header
{
	Header(): deflate(false), acknowledge(false), id(0), partNumber(0), totalParts(0) {}

	inline void Pack(char* p)
	{
		*p++ = 'G';
		*p++ = 'N';
		*p++ = 'D';

		*p = 0;
		if(deflate) *p |= 0x01;
		if(acknowledge) *p |= 0x02;
		p++;

		*p++ = (id >> 8) & 0xFF;
		*p++ = id & 0xFF;

		*p++ = partNumber;
		*p = totalParts;
	}

	inline bool Unpack(const char* p, uint length)
	{
		if(length < 8) return false;
		if(p[0] != 'G' || p[1] != 'N' || p[2] != 'D') return false;

		p += 3;

		deflate = *p & 0x1;
		acknowledge = *p & 0x2;
		++p;

		id = *p++ << 8;
		id += *p++;

		partNumber = *p++;
		totalParts = *p;
		return true;
	}

	bool deflate;
	bool acknowledge;
	PacketID id; //sequence
	uint8 partNumber;
	uint8 totalParts;
};

} //namespace udp

#endif //UDP_HPP


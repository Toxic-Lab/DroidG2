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

#ifndef HEADER_HPP
#define HEADER_HPP

#include "packettype.hpp"
#include <stdexcept>
#include <cstring>

namespace G2 {

struct ControlByte
{
	ControlByte(): lengthSize(0), nameSize(0), compound(false), bigendian(false) {}
	ControlByte(char b) { Decode(b); }

	void Decode(char b)
	{
		lengthSize = 	(b & 0xC0) >> 6;
		nameSize = 	((b & 0x38) >> 3) + 1;
		compound = 	b & 0x04;
		bigendian = 	b & 0x02;
	}
	
	char Encode() const;
	std::size_t lengthSize;
	std::size_t nameSize;
	bool compound;
	bool bigendian;
};

struct Header 
{
	void Decode(const char* p, uint size)
	{
		if(size == 0) throw std::range_error("Control byte not found");

		const char cb = *p;
		int lenLen = (cb & 0xC0) >> 6;
		int nameLen = ((cb & 0x38) >> 3) + 1;
		compound = (cb & 0x04);
		endian = (cb & 0x02);
	
		headerSize = 1 + lenLen + nameLen;

		if(headerSize > size) throw std::range_error("No space for header");
	
		dataSize = 0;
		std::memcpy(&dataSize, p + 1, lenLen);

		type = 0;
	
		for(int i = 0; i < nameLen; i++) 
			((char*)(&type))[i] = *(p + 1 + lenLen + i);

		packetSize = dataSize + headerSize;
	}

	PacketType type;
	bool compound;
	bool endian;
	uint32 dataSize;
	uint32 headerSize;
	uint packetSize;
};

inline char LenLength(int len)
{
	char lenlen = 0;
	while(len)
	{
		len >>= 8;
		++lenlen;
	}
	return lenlen;
}

inline char NameLength(PacketType t)
{
	return 	((t & 0x000000ff) > 0) + 
	((t & 0x0000ff00) > 0) + 
	((t & 0x00ff0000) > 0) + 
	((t & 0xff000000) > 0) + 
	(((t >> 32) & 0x000000ff) > 0) + 
	(((t >> 32) & 0x0000ff00) > 0) + 
	(((t >> 32) & 0x00ff0000) > 0) + 
	(((t >> 32) & 0xff000000) > 0);
}


} //namespace G2


#endif //HEADER_HPP




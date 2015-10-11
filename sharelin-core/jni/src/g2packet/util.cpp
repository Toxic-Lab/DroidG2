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

#include "util.hpp"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include "../staticname.hpp"

namespace G2 {

void PrintHex(const char* buf, int len)
{
 	for(int i = 0; i < len; i++)
		std::cout << std::hex << int(buf[i] & 0xff) << ' ';
	std::cout << std::dec;
		//std::printf("%02X ", buf[i]);
}

void Print(Reader r, int depth)
{
	while(r.Next())
	{
		std::cout << std::setw(depth * 2) << ' ' << StaticName::ToString(r.Type()) << std::endl;
		Print(r.Follow(), depth + 1);
	}
	std::cout << std::setw(depth * 2) << ' ' << r.BytesLeft() << ": ";
       	PrintHex(r.Data(), r.BytesLeft());
	std::cout << std::endl;
}

void Print(const Packet& pk)
{
	Reader r(pk);
	std::cout << StaticName::ToString(r.Type()) << std::endl;
	Print(r, 0);
}

} //namespace G2

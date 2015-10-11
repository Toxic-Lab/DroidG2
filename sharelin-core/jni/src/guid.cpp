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

#include "guid.hpp"
#include <cstdlib>
#include <algorithm>

GUID& GUID::operator=(const GUID& g)
{
	if(&g != this) CopyFrom(g);

	return *this;
}

void GUID::Clear()
{
	for(int i = 0; i < 16; ++i)
		bytes[i] = 0;
}

GUID::GUID(const char* p)
{
	Assign(p);
}

void GUID::Generate()
{
	for(int i = 0; i < 16; ++i)
		bytes[i] = std::rand() & 0xFF;
}

void GUID::CopyFrom(const GUID& g)
{
	for(int i = 0; i < 16; ++i)
		bytes[i] = g.bytes[i];
}

bool GUID::Empty() const 
{
	return !(
			bytes[0] || bytes[1] || bytes[2] || bytes[3] ||
			bytes[4] || bytes[5] || bytes[6] || bytes[7] ||
			bytes[8] || bytes[9] || bytes[10] || bytes[11] ||
			bytes[12] || bytes[13] || bytes[14] || bytes[15]
		);
}	

bool operator< (const GUID& g1, const GUID& g2)
{
	return std::lexicographical_compare(g1.Get(), g1.Get() + 16, g2.Get(), g2.Get() + 16);
}

bool operator== (const GUID& g1, const GUID& g2)
{
	return std::equal(g1.Get(), g1.Get() + 16, g2.Get());
}

bool operator!= (const GUID& g1, const GUID& g2)
{
	return !(g1 == g2);
}


/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009  Andrey Stroganov

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

#ifndef STATICNAME_HPP
#define STATICNAME_HPP

#include <string>
#include "types.hpp"

namespace StaticName
{
	template <char a = 0, char b = 0, char c = 0, char d = 0, char e = 0, char f = 0, char g = 0, char h = 0> 
	struct Encoder
	{
		enum { 
			result = uint64(a) | (uint64(b) << 8) | 
				(uint64(c) << 16) | (uint64(d) << 24) | 
				(uint64(e) << 32) | (uint64(f) << 40) | 
				(uint64(g) << 48) | (uint64(h) << 56) 
		};
	};

	uint64 FromString(const std::string& str);
	std::string ToString(uint64 nm);
}

#endif //STATICNAME_HPP

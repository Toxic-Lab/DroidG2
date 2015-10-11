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

#ifndef CONVHEX_HPP
#define CONVHEX_HPP

#include "common.hpp"

namespace Conv {

class Hex 
{
public:

	static std::size_t EncodeSize(std::size_t source)
	{
		return source * 2;
	}

	static std::size_t DecodeSize(std::size_t source)
	{
		if(source % 2 != 0) 
			throw Error("Input size");
		return source / 2;
	}

	template <typename InputIterator, typename OutputIterator>
	static void Encode(InputIterator begin, InputIterator end, OutputIterator out)
	{
		static const char* digits = "0123456789abcdef";
		for(InputIterator i = begin; i != end; ++i)
		{
			*out++ = digits[(*i & 0xF0) >> 4];
			*out++ = digits[*i & 0x0F];
		}
	}


	template <typename InputIterator, typename OutputIterator>
	static void Decode(InputIterator begin, InputIterator end, OutputIterator out)
	{
		for(InputIterator i = begin; i != end;)
		{
			const byte b1 = HexByte(*i++);
			if(i == end) throw Error("Input size");
			const byte b2 = HexByte(*i++);
			*out++ = (b1 << 4) | b2;
		}
	}

private:
	static inline byte HexByte(char c)
	{
		switch(c)
		{
			case '0': return 0;
			case '1': return 1;
			case '2': return 2;
			case '3': return 3;
			case '4': return 4;
			case '5': return 5;
			case '6': return 6;
			case '7': return 7;
			case '8': return 8;
			case '9': return 9;
			case 'a': case 'A': return 10;
			case 'b': case 'B': return 11;
			case 'c': case 'C': return 12;
			case 'd': case 'D': return 13;
			case 'e': case 'E': return 14;
			case 'f': case 'F': return 15;
			default: throw Error("Bad hex digit");
		}
	}

};

} //namespace Conv 

#endif //CONVHEX_HPP


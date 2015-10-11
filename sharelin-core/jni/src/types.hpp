/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008  Andrey Stroganov <savthe@gmail.com>

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

#ifndef TYPES_HPP
#define TYPES_HPP 

#include <boost/cstdint.hpp>

typedef boost::int8_t int8;
typedef boost::uint8_t uint8;
typedef boost::int16_t int16;
typedef boost::uint16_t uint16;
typedef boost::int32_t int32;
typedef boost::uint32_t uint32;
typedef boost::int64_t int64;
typedef boost::uint64_t uint64;
typedef unsigned int uint;
typedef unsigned char byte;
typedef uint32 vendor_t;
typedef uint32 ID;
typedef uint32 file_offset_t;

const ID INVALID_ID = 0;

#endif //TYPES_HPP

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

#ifndef IDENTITY_HPP
#define IDENTITY_HPP
#include "types.hpp"

namespace Ident
{
	const char name[] = "Sharelin";
	const char tag[] = "0.2.0";
	const char version[] = "0.2.0";
	const char fullname[] = "Sharelin-0.2.0";
	const unsigned int birthday = 20100806;
	const char vendor[] = "SHLN";
	const uint32 vid = uint32('S') << 24 | uint32('H') << 16 | uint32('L') << 8 | uint32('N');
}

#endif //IDENTITY_HPP

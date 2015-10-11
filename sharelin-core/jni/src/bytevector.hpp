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

#ifndef BYTEVECTOR_HPP
#define BYTEVECTOR_HPP

#include <vector>
#include <cstring>

class ByteVector: public std::vector<char>
{
public:
	void assign(const char* buffer, unsigned int sz)
	{
		resize(sz);
		std::memcpy(data(), buffer, sz);
	}

	void append(const char* buffer, unsigned int sz)
	{
		std::size_t offset = size();
		resize(size() + sz);
		std::memcpy(data() + offset, buffer, sz);
	}

	inline char* data() { return &(*this)[0]; }
	inline const char* data() const { return &(*this)[0]; }
};

#endif //BYTEVECTOR_HPP


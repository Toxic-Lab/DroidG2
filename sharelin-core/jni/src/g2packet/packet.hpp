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

#ifndef G2_PACKET_HPP
#define G2_PACKET_HPP

#include "../bytepool.hpp"

namespace G2 {

class Packet
{
public:
	typedef ByteVector Buffer;
	Packet();
	Packet(const Packet& pk);
	Packet& operator=(const Packet& pk);
	~Packet();

	const char* Bytes() const;
	unsigned int Size() const;
	Buffer* GetBuffer() const;

private:
	Buffer* pBuffer_;
	static BytePool pool_;
};

} //namespace G2

#endif //G2_PACKET_HPP

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

#ifndef WRITER_HPP
#define WRITER_HPP

#include <stack>
#include <string>

#include "packet.hpp"
#include "packettype.hpp"

namespace G2 {

class Writer
{
	struct Details
	{
		Details(): type(0), headerOffset(0), dataOffset(0), compound(false),  hasPayload(false) {}
		PacketType type;
		uint headerOffset;
		uint dataOffset;
		bool compound;
		bool hasPayload;
	};
public:
	Writer(Packet& pk):
		pBuffer_(pk.GetBuffer()),
		offset_(0)
	{ 
	}

	void Begin(PacketType type);
	void Close(PacketType type);
	void Write(const char* buffer, uint length);
	void Write(const std::string&);

	template <typename T>
	void Pod(const T& val)
	{
	//	BOOST_STATIC_ASSERT(boost::is_pod<T>::value);
		Write(reinterpret_cast<const char*>(&val), sizeof(T));
	}

protected:
	void Allocate(uint);
	/*
	inline void Verify(uint n)
	{
		if(offset_ + n >= packet_.reserved_) throw std::range_error("Out or reserved space");
	}
	*/

	void Put(char c);
	void Put(const char* buf, int length);
	char* Bytes(uint);

	Packet::Buffer* pBuffer_;
	uint offset_;
	std::stack<Details> stack_;
};

} // namespace G2

#endif //WRITER_HPP



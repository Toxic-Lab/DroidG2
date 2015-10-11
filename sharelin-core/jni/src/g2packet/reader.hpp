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

#ifndef READER_HPP
#define READER_HPP

#include "error.hpp"
#include "packet.hpp"
#include "header.hpp"

namespace G2 {

class Reader
{
public:
	Reader(): data_(0), size_(0), offset_(0), compound_(false) {} 
	Reader(const Packet& pk)
	{
		Assign(pk.Bytes(), pk.Size());
	}

	bool Next();

	const char* Data(); 
	uint BytesLeft();

	void Skip(uint length);
	void Read(char* out, uint length, uint off = 0);
	bool Contains(PacketType) const;

	template <typename T>
	T Pod(uint off = 0)
	{
		T val;
		//BOOST_STATIC_ASSERT(boost::is_pod<T>::value);
		Read(reinterpret_cast<char*>(&val), sizeof(T), off);
		return val;
	}

	PacketType Type() const
	{
		if(!compound_ && offset_) throw std::range_error("Not at packet");

		return header_.type;
	}

	Reader Follow();

	void Assign(const char* data, uint length);
	bool AtPacket() const { return compound_ && offset_ && offset_ < size_; }
	void SkipPackets();

protected:
	inline void Advance(unsigned int n)
	{
		if(offset_ + n > size_) throw BadPacket("offset > size");
		offset_ += n;
	}

	const char* data_;
	uint size_;
	uint offset_;
	bool compound_;
	Header header_;
};

} //namespace G2

#endif //READER_HPP



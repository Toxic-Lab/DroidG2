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

#include "reader.hpp"
#include <cassert>
#include <cstring>

namespace G2
{

const char* Reader::Data()
{
	if(AtPacket()) return Follow().Data();
	SkipPackets();
	return data_ + offset_;
}

void Reader::SkipPackets()
{
	while(Next());
}

uint Reader::BytesLeft()
{
	if(AtPacket()) return Follow().BytesLeft();
	SkipPackets();
	return size_ - offset_;
}

bool Reader::Next()
{
	if(offset_ == size_) return false;
	if(offset_ == 0) Advance(header_.headerSize);
	else if(compound_) Advance(header_.packetSize);
	else return false;

	assert(offset_ <= size_);
	if(offset_ == size_) return false;
	if(offset_ && compound_ && *(data_ + offset_) == 0)
	{
		Advance(1);
		assert(offset_ <= size_);
		compound_ = false;
	}
	assert(offset_ <= size_);
	if(compound_) header_.Decode(data_ + offset_, size_ - offset_);
	return compound_;
}

bool Reader::Contains(PacketType type) const 
{
	Reader r;
	r.Assign(data_, size_);
	while(r.Next())
		if(r.Type() == type) return true;
	return false;
}

void Reader::Skip(uint length)
{
	SkipPackets();
	if(BytesLeft() < length) 
	{
//		std::cout << BytesLeft() << " " << length << std::endl;
		throw std::range_error("bad range");
	}

	Advance(length);
}

void Reader::Read(char* out, uint length, uint off)
{
	if(AtPacket()) Follow().Read(out, length, off);
	else
	{
		Skip(off);
		if(BytesLeft() < length) 
			throw std::range_error("bad range");

		std::memcpy(out, Data(), length);
		Advance(length);
	}
}

Reader Reader::Follow()
{
	if(offset_ == 0) Next();
	if(!compound_) throw std::runtime_error("Not a packet");

	Reader r;
	assert(offset_ <= size_);
	r.Assign(data_ + offset_, size_ - offset_);
	return r;
}

void Reader::Assign(const char* data, uint length)
{
	data_ = data;
	offset_ = 0;

	header_.Decode(data, length);

	size_ = header_.packetSize;
	if(size_ > length) throw std::range_error("No space for packet");
	compound_ = header_.compound;
}

} //namespace G2

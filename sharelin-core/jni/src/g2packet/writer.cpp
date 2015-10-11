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

#include <stdexcept>
#include <cassert>

#include "writer.hpp"
#include "header.hpp"

namespace G2 {

void Writer::Begin(PacketType type)
{
	if( !stack_.empty() )
	{
		if( stack_.top().hasPayload ) throw std::logic_error("Packet after payload");
		//setting parent compound
		*Bytes(stack_.top().headerOffset) |= 0x04;
		stack_.top().compound = true;
	}
	
	//setting up new offset
	Details d;
	d.headerOffset = offset_;
	d.dataOffset = offset_ + 1 + 3 + NameLength(type);
	d.type = type;
	stack_.push(d);

	//we are at control byte now
	char control = 0;
	control |= (NameLength(type) - 1) << 3;
	Put(control);

	Put(0);
	Put(0);
	Put(0);

	//go to the name field
	char c;
	for(uint i = 0; (c = ((char*)(&type))[i] ) && (i < sizeof(PacketType)); ++i)
	{
		Put(c);
	}


	//we are on the first byte after the header
}

void Writer::Close(PacketType type)
{
	Details my = stack_.top();
	if(type != my.type) throw std::logic_error("Closed packet");

	//move data on few bytes of lenlen and write lenlen
	uint dataLength = offset_ - my.dataOffset;
	uint lenlen = LenLength(dataLength);
	const uint over = 3 - lenlen;
	for(uint off = my.headerOffset + 1 + lenlen; off + over< offset_; ++off)
	{
		*Bytes(off) = *Bytes(off + over);
	}
	offset_ -= over;

	assert(lenlen < 4);
	//write lenlen field
	*Bytes(my.headerOffset) |= lenlen << 6;

	//write len field
	for(uint off = 0; off < lenlen; ++off)
		*Bytes(my.headerOffset + 1 + off) = reinterpret_cast<const char*>(&dataLength)[off];

	//restore myOffset_
	stack_.pop();
	//packet_.size_ = offset_;

	//root is closed
	assert(offset_ <= pBuffer_->size());
	if(stack_.empty())
		pBuffer_->erase(pBuffer_->begin() + offset_, pBuffer_->end());
}

void Writer::Write(const std::string& s)
{
	Write( s.c_str(), s.size() );
}

void Writer::Write(const char* buffer, uint length)
{
	if(stack_.top().compound && !stack_.top().hasPayload)
		Put(0);

	stack_.top().hasPayload = true;

	Put(buffer, length);
}

void Writer::Put(char c)
{
	Allocate(1);
	*Bytes(offset_) = c;
	++offset_;

}

void Writer::Put(const char* buf, int length)
{
	//std::cout << offset_ << " " << packet_.data_.size() << std::endl;
	assert(offset_ <= pBuffer_->size());
	Allocate(length);
	std::memcpy(Bytes(offset_), buf, length);
	offset_ += length;
}

void Writer::Allocate(uint size)
{
	if(offset_ + size > pBuffer_->size())
		pBuffer_->resize(offset_ + 2 * size);
}

char* Writer::Bytes(uint off) 
{ 
	assert(off < pBuffer_->size()); 
	assert(off <= offset_); 
	return &(*pBuffer_)[0] + off; 
}

} //namespace G2

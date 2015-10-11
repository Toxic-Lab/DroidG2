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

    

    Base32 encoding/decoding routines are from CyoEncode by Graham Bull:
//////////////////////////////////////////////////////////////////////
// Developed by Graham Bull for Cyotec Systems Limited.
// http://www.cyotec.com
//
// Copyright (c) 2004 by Cyotec Systems Limited. All Rights Reserved.
//
// This source code may be used, modified, compiled, duplicated, and/or
// distributed without restriction provided this copyright notice remains intact.
// Cyotec Systems Limited and/or its employees cannot be held responsible for any
// direct or indirect damage or loss of any kind that may result from using this
// code, and provides no warranty, guarantee, or support.
//////////////////////////////////////////////////////////////////////

*/

#ifndef CONVBASE32_HPP
#define CONVBASE32_HPP

#include "common.hpp"
#include <assert.h>
#include <iostream>

namespace Conv {

class Base32 
{
public:

template <typename InputIterator, typename OutputIterator>
static void Decode(InputIterator begin, InputIterator end, OutputIterator out)
{
	const std::size_t SIZE = 8;
	char buffer[SIZE];
	std::size_t padding = 0;

	for(InputIterator i = begin; i != end;)
	{
		for(std::size_t index = 0; index < SIZE; ++index)
		{
			byte b = 32;
		        if(i != end)
				b = Base32ToDecimal(*i++);

			padding += b == 32 ? 1 : 0;
			if(padding && b != 32) throw Error("format");
			buffer[index] = b;
		}

        	*out++ = ((buffer[0] & 0x1F) << 3) | ((buffer[1] & 0x1C) >> 2);
		if(padding <= 4) *out++ = ((buffer[1] & 0x03) << 6) | ((buffer[2] & 0x1F) << 1) | ((buffer[3] & 0x10) >> 4);
		if(padding <= 3) *out++ = ((buffer[3] & 0x0F) << 4) | ((buffer[4] & 0x1E) >> 1);
		if(padding <= 1) *out++ = ((buffer[4] & 0x01) << 7) | ((buffer[5] & 0x1F) << 2) | ((buffer[6] & 0x18) >> 3);
		if(padding == 0) *out++ = ((buffer[6] & 0x07) << 5) | (buffer[7] & 0x1F);
	}
}

enum EncodeType {standart, compact};

template <typename InputIterator, typename OutputIterator>
static void Encode(InputIterator begin, InputIterator end, OutputIterator out, EncodeType etype = standart)
{
	const std::size_t SIZE = 5;
	char buffer[SIZE];

	for(InputIterator i = begin; i != end;)
	{
		std::size_t index = 0;
		for(index = 0; index < SIZE && i != end; ++i, ++index) 
			buffer[index] = *i;

		byte v[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        	switch (index)
        	{
        		case 5:
            			v[7]  =  (buffer[ 4 ] & 0x1f);
            			v[6]  = ((buffer[ 4 ] & 0xe0) >> 5);

        		case 4:
            			v[6] |= ((buffer[ 3 ] & 0x03) << 3);
            			v[5]  = ((buffer[ 3 ] & 0x7c) >> 2);
            			v[4]  = ((buffer[ 3 ] & 0x80) >> 7);

        		case 3:
            			v[4] |= ((buffer[ 2 ] & 0x0f) << 1);
            			v[3]  = ((buffer[ 2 ] & 0xf0) >> 4);

        		case 2:
            			v[3] |= ((buffer[ 1 ] & 0x01) << 4);
            			v[2]  = ((buffer[ 1 ] & 0x3e) >> 1);
            			v[1]  = ((buffer[ 1 ] & 0xc0) >> 6);

        		case 1:
            			v[1] |= ((buffer[ 0 ] & 0x07) << 2);
           		 	v[0]  = ((buffer[ 0 ] & 0xf8) >> 3);
            			break;

			default: assert(!"Something wrong in Base32::EncodeRound::1");
        	}

		for(int i = 0; i < 8; ++i)
			if(v[i] > 31) throw Error("validate error");

		switch (index)
		{
			case 1: v[2] = v[3] = 32;
			case 2: v[4] = 32;
			case 3: v[5] = v[6] = 32;
			case 4: v[7] = 32;
			case 5: break;
			default: assert(!"Something wrong in Base32::EncodeRound::2");
		}

    		const char* base32Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567=";
		for(int i = 0; i < 8; ++i)
		{
			if((etype == compact && v[i] < 32) || (etype == standart)) 
				*out++ = base32Table[v[i]];
			else break;
		}
	}	
}

private:

static inline byte Base32ToDecimal(byte b)
{
	if(b >= 'A' && b <= 'Z') b -= 'A';
	else if(b >= 'a' && b <= 'z') b -= 'a';
	else if(b >= '2' && b <= '7') b = b - '2' + 26;
	else if(b == '=') b = 32;
	else throw Error("Bad base32 number");
	return b;
}



/*
class Decoder
{
public:
	Decoder(): index_(0), padding_(0) {}

	template <typename InputIterator, typename OutputIterator>
	void Encode(InputIterator begin, InputIterator end, OutputIterator out)
	{
		for(InputIterator i = begin; i != end; ++i)
		{
			buffer_[index_++] = Base32ToDecimal(*i);

			if((padding_ && *i != '=') || padding_ > 6) 
				throw Error("Malformated");

			if(*i == '=') ++padding_;
		
			if(index_ == BUFFER_SIZE)
				DoRound(out);
			index_ %= BUFFER_SIZE;
		}

		if(index_)
		{
			while(index_ < BUFFER_SIZE)
			{
				buffer_[index_++] = 32;
				++padding_;
			}
			DoRound(out);
		}
	}

private:
	template <typename IteratorOut>
	void DoRound(IteratorOut& out)
	{
		assert(index_ == BUFFER_SIZE);

        	*out++ = ((buffer_[0] & 0x1F) << 3) | ((buffer_[1] & 0x1C) >> 2);
		if(padding_ <= 4) *out++ = ((buffer_[1] & 0x03) << 6) | ((buffer_[2] & 0x1F) << 1) | ((buffer_[3] & 0x10) >> 4);
		if(padding_ <= 3) *out++ = ((buffer_[3] & 0x0F) << 4) | ((buffer_[4] & 0x1E) >> 1);
		if(padding_ <= 1) *out++ = ((buffer_[4] & 0x01) << 7) | ((buffer_[5] & 0x1F) << 2) | ((buffer_[6] & 0x18) >> 3);
		if(padding_ == 0) *out++ = ((buffer_[6] & 0x07) << 5) | (buffer_[7] & 0x1F);
	}

	enum {BUFFER_SIZE = 8};
	byte buffer_[BUFFER_SIZE];
	byte index_;
	byte padding_;
};
*/
/*
template <class Condition>
class GenericEncoder
{
public:
	GenericEncoder(): index_(0) {}

	template <typename InputIterator, typename OutputIterator>
	void Encode(InputIterator begin, InputIterator end, OutputIterator out)
	{
		for(InputIterator i = begin; i != end; ++i)
		{
			buffer_[index_++] = *i;
			if(index_ == BUFFER_SIZE)
				DoRound(out);
			index_ %= BUFFER_SIZE;
		}	

		if(index_) DoRound(out);
	}

private:

	template <typename IteratorOut>
	void DoRound(IteratorOut& out)
	{
		byte v[8] = {0, 0, 0, 0, 0, 0, 0, 0};
			//std::cout << int(index_) << std::endl;
        	switch (index_)
        	{
        		case 5:
            			v[7]  =  (buffer_[ 4 ] & 0x1f);
            			v[6]  = ((buffer_[ 4 ] & 0xe0) >> 5);

        		case 4:
            			v[6] |= ((buffer_[ 3 ] & 0x03) << 3);
            			v[5]  = ((buffer_[ 3 ] & 0x7c) >> 2);
            			v[4]  = ((buffer_[ 3 ] & 0x80) >> 7);

        		case 3:
            			v[4] |= ((buffer_[ 2 ] & 0x0f) << 1);
            			v[3]  = ((buffer_[ 2 ] & 0xf0) >> 4);

        		case 2:
            			v[3] |= ((buffer_[ 1 ] & 0x01) << 4);
            			v[2]  = ((buffer_[ 1 ] & 0x3e) >> 1);
            			v[1]  = ((buffer_[ 1 ] & 0xc0) >> 6);

        		case 1:
            			v[1] |= ((buffer_[ 0 ] & 0x07) << 2);
           		 	v[0]  = ((buffer_[ 0 ] & 0xf8) >> 3);
            			break;

			default: assert(!"Something wrong in Base32::EncodeRound::1");
        	}

		for(int i = 0; i < 8; ++i)
			if(v[i] > 31) throw Error("validate error");

		switch (index_)
		{
			case 1: v[2] = v[3] = 32;
			case 2: v[4] = 32;
			case 3: v[5] = v[6] = 32;
			case 4: v[7] = 32;
			case 5: break;
			default: assert(!"Something wrong in Base32::EncodeRound::2");
		}

    		const char* base32Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567=";
		for(int i = 0; i < 8; ++i)
		{
			if(Condition::Eval(v[i])) 
				*out++ = base32Table[v[i]];
			else break;
		}
	}


	enum {BUFFER_SIZE = 5};
	byte index_;
	byte buffer_[BUFFER_SIZE];
};

class Compact
{
public:
	static bool Eval(byte b) { return b < 32; }
};

class Standart
{
public:
	static bool Eval(byte b) { return true; }
};

typedef GenericEncoder<Compact> CompactEncoder;
typedef GenericEncoder<Standart> Encoder;
*/

}; //class Base32

} //namespace Conv 

#endif //CONVBASE32_HPP


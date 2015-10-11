/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008  Andrey Stroganov

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

#include <iostream>
#include "queryhashtable.hpp"
#include <assert.h>

/**************************************************************
Took some of QuerryHashTable code from Shareaza.
http://shareaza.sourceforge.com

sav.
**************************************************************/

namespace Share {

uint QueryHashTable::HashBits() const
{
	return hash_bits;
}

const char* QueryHashTable::Bytes() const
{
	return p_hashtable;
}

QueryHashTable::QueryHashTable()
{
	hash_bits = 20;
	p_hashtable = 0;
	table_size = ((1u << hash_bits) + 31) / 8;
	p_hashtable = new char[table_size];
	Reset();
}

uint QueryHashTable::Size() const
{
	assert(SizeInBits() / 8 <= table_size);
	return SizeInBits() / 8;
}

uint32 QueryHashTable::SizeInBits() const
{
	return 1u << hash_bits;
}

void QueryHashTable::Reset()
{
	for(uint i = 0; i < table_size; i++)
		p_hashtable[i] = 0xFF;
}

QueryHashTable::~QueryHashTable()
{
	if(p_hashtable) delete[] p_hashtable;
	p_hashtable = 0;
}
/*void QueryHashTable::DirtyWorkaround()
{
	for(int i = 0; i < table_size; i++)
		p_hashtable[i] = 0xFF - p_hashtable[i];
}
*/
uint32 QueryHashTable::BuildHash(const char* p_str, uint32 len, uint32 bits)
{
	uint32 result	= 0;
	int nbyte	= 0;

	for (; len ; len--, p_str++ )
	{
		int val = tolower( *p_str ) & 0xFF;

		val = val << ( nbyte * 8 );
		nbyte = ( nbyte + 1 ) & 3;

		result = result ^ val;
	}

	return BuildHashNumber( result, bits );
}

uint32 QueryHashTable::BuildHashNumber(uint32 num, int bits)
{
	uint64 product	= (uint64)num * (uint64)0x4F1BBCDC;
	uint64 hash	= product << 32;
	hash = hash >> ( 32 + ( 32 - bits ) );
	return (uint32) hash;
}

void QueryHashTable::Add(const char* p_str, int len)
{
	//return;
	uint32 hash_num	= BuildHash( p_str, len, hash_bits );
	assert((hash_num >> 3) < table_size);
	char* p	= p_hashtable + ( hash_num >> 3 );
	char mask	= byte( 1 << ( hash_num & 7 ) );

	if ( *p & mask )
	{
		//m_nCount++;
		*p &= ~mask;
	}
}

void QueryHashTable::Add(const std::string& s)
{
	Add(s.c_str(), s.size());
}

} //namespace Share

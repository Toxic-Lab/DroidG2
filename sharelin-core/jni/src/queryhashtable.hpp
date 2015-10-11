/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010 Andrey Stroganov <savthe@gmail.com>

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

#ifndef QUERYHASHTABLE_HPP
#define QUERYHASHTABLE_HPP

/**************************************************************
Took some of QuerryHashTable code from Shareaza.
http://shareaza.sourceforge.com

Sav.
**************************************************************/

#include "types.hpp"

namespace Share {

class QueryHashTable
{
public:
	QueryHashTable();
	~QueryHashTable();
	void Reset();
	void Add(const std::string& s);
	uint HashBits() const ;
	const char* Bytes() const;
	uint Size() const;
	uint32 SizeInBits() const;
//	void DirtyWorkaround();

private:
	char* p_hashtable;
	uint32 table_size;
	uint32 BuildHash(const char* p_str, uint32 len, uint32 bits);
	uint32 BuildHashNumber(uint32 num, int bits);
	void Add(const char* p_str, int len);
	int hash_bits;
};

} //namespace Share

#endif //QUERYHASHTABLE_HPP

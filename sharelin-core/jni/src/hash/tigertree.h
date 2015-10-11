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

#ifndef TIGERTREE_H
#define TIGERTREE_H

#include <vector>
#include <string>
#include "types.h"
	
const size_t TIGER_BLOCK_SIZE = 1024;
const size_t TIGER_HASH_SIZE = 24;

struct tt_empty{};

class TigerNode {
public:
	TigerNode() {}
	TigerNode& operator=(const TigerNode& tn);
	TigerNode(const TigerNode& tn);

	byte hash[TIGER_HASH_SIZE];
};


class TigerTree {
public:
	TigerTree();
	TigerTree(const TigerTree& tt);
	TigerTree& operator=(const TigerTree& tt);
	uint64 MinimalRange() const;
	void Init(uint64 sz, uint h);
	const byte* Root() const;
	bool Validate();
	void Clear();
	bool FromBytes(const ByteVector& v, uint64 size);
	void ToBytes(ByteVector& v);
	bool IsValidRange(int64 offset, int64 length, uint64* p_hash);
	bool Empty() const;
	void CopyFrom(const TigerTree& tt);
	~TigerTree();

private:
	std::vector <TigerNode*> tree_;
	uint totalBlocks_;
	uint64 blockSize_;
};

#endif //TIGERTREE_H


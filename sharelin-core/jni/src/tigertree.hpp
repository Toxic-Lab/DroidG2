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

#ifndef TIGERTREE_HPP
#define TIGERTREE_HPP

#include <vector>
#include <string>
#include "types.hpp"
#include "hashes.hpp"
#include "filerange.hpp"
	
namespace Hashes {

const size_t TIGER_BLOCK_SIZE = 1024;
const size_t TIGER_HASH_SIZE = 24;

struct tt_empty{};

class TigerNode {
public:
	TigerNode() {}
	TigerNode& operator=(const TigerNode& tn);
	TigerNode(const TigerNode& tn);

	char hash[TIGER_HASH_SIZE];
};

typedef std::vector<char> ByteVector; //temporary

class TigerTree {
public:
	TigerTree();
	TigerTree(const TigerTree& tt);
	TigerTree& operator=(const TigerTree& tt);
	file_offset_t MinimalRange() const;
	void Init(file_offset_t sz, uint h);
	Hashes::TTR Root() const;
	Hashes::TTR Get(FileRange) const;
	bool Validate();
	void Clear();
	bool FromBytes(const ByteVector& v, file_offset_t size);
	void ToBytes(ByteVector& v);
	bool IsValidRange(file_offset_t offset, file_offset_t length, file_offset_t* p_hash);
	bool Empty() const;
	void CopyFrom(const TigerTree& tt);
	~TigerTree();

private:
	std::vector <TigerNode*> tree_;
	uint totalBlocks_;
	file_offset_t blockSize_;
};

} //namespace Hashes

#endif //TIGERTREE_HPP


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

#include "tigertree.hpp"
#include <iostream>
#include <assert.h>
#include "hash/tiger.h"
#include <unistd.h>
#include <fcntl.h>
#include "hash/ttr.h"
#include <sys/stat.h>
#include "hashes.hpp"
#include <stdio.h>
#include "stdexcept"
#include <cstring>
//#include <android/log.h>

void Tiger(const char* buf, std::size_t len, char* hash)
{
	::tiger((word64*)buf, len, (word64*)hash);
}

namespace Hashes {

const uint BLOCK_SIZE = 1024; //1024;
const uint TIGER_SIZE = 24;
const int INVALID_NODE = 0;


/*void TigerTree::Hasher::Init(file_offset_t fileSize, uint depth)
{
	offset = 0;
	tree_.Init(fileSize, depth);
}

void TigerTree::Hasher::Update(const char* buf, size_t len)
{
	size_t i = 0;
	while(i < len)
	{
		const size_t n = std::min(len - i, TIGER_BLOCK_SIZE - offset_);
		memcpy(block_ + offset, buf + i, n);
		offset_ += n;
		i += n;
		if(offset == TIGER_BLOCK_SIZE)
		{
			OnBlock();
			offset = 0;
		}
	}
}
*/

Hashes::TTR TigerTree::Get(FileRange r) const
{
	if(Empty()) throw std::logic_error("Tree is empty");

	int64 parent = (tree_.size() / 2 + r.Offset() / blockSize_);
	int64 len = r.Length() / blockSize_ + (r.Length() % blockSize_ ? 1 : 0);
	while(len > 1) {
		parent /= 2;
		len = len % 2 ? len / 2 + 1 : len / 2;
	}
	assert(tree_[parent]);
	

	return Hashes::TTR(tree_[parent]->hash);
	
}

file_offset_t TigerTree::MinimalRange() const
{
	return blockSize_;
}

bool TigerTree::IsValidRange(file_offset_t offset, file_offset_t length, file_offset_t* p_hash)
{
	int64 parent = (tree_.size() / 2 + offset / blockSize_);
	int64 len = length / blockSize_ + (length % blockSize_ ? 1 : 0);
	while(len > 1) {
		parent /= 2;
		len = len % 2 ? len / 2 + 1 : len / 2;
	}
	assert(tree_[parent]);
	
	return 0 == std::memcmp((char*)p_hash, (char*)tree_[parent]->hash, 24);
}

void TigerTree::Init(file_offset_t fileSize, uint maxDepth)
{
	totalBlocks_ = (fileSize % TIGER_BLOCK_SIZE) ? fileSize / TIGER_BLOCK_SIZE + 1 : fileSize / TIGER_BLOCK_SIZE;
	
	uint depth = 1;
	for(uint i = 1; i < totalBlocks_; i *= 2) 
		++depth; 
	
	blockSize_ = TIGER_BLOCK_SIZE;
	while(maxDepth && maxDepth < depth) 
	{
		--depth;
		blockSize_ *= 2;
		totalBlocks_ = totalBlocks_ % 2 ? (totalBlocks_ + 1) / 2 : totalBlocks_ / 2;
	}

	uint nodes = 0;
	for(uint i = 1, j = 1; i <= depth; ++i, j *= 2) 
		nodes += j;
	
	//std::cout << "size: " << sz << " height: " << height << " nodes: " << nodes << " bottom:"<<bottom << " blocks: " << total_blocks << std::endl; 
	
	tree_.resize(nodes);
	tree_[0] = new TigerNode;
	for(uint i = 1; i < nodes; ++i) 
		tree_[i] = 0;
	
	for(uint i = 0; i < totalBlocks_; ++i)
	{
		uint node = tree_.size() / 2 + i;
		do
		{
			if(tree_[node] == INVALID_NODE)
				tree_[node] = new TigerNode;
			node = (node - 1) / 2;
		} while(node);
	}

	/*tree[0] = new TigerNode;
	
	for(uint i = 0; i < total_blocks; i++) {
		tree[bottom + i] = new TigerNode;
		int parent = (bottom + i - 1) / 2;
		while(parent && !tree[parent]) {
			tree[parent] = new TigerNode;
			parent = (parent - 1) / 2;
		}
	}
	*/

}

TigerTree::TigerTree() 
{
	totalBlocks_ = 0;
	blockSize_ = 0;
}

TigerNode& TigerNode::operator=(const TigerNode& tn)
{
	if(this != &tn) 
		std::copy(tn.hash, tn.hash + TIGER_HASH_SIZE, hash);
	
	return *this;
}
TigerNode::TigerNode(const TigerNode& tn)
{
	std::copy(tn.hash, tn.hash + TIGER_HASH_SIZE, hash);
}

void TigerTree::CopyFrom(const TigerTree& tt)
{
	Clear();
	totalBlocks_ = tt.totalBlocks_;
	blockSize_ = tt.blockSize_;
	tree_.reserve(tt.tree_.size());
	for(size_t i = 0; i < tt.tree_.size(); ++i)
		tree_.push_back(tt.tree_[i] != INVALID_NODE ? new TigerNode(*tt.tree_[i]) : 0);
}

TigerTree& TigerTree::operator=(const TigerTree& tt)
{
	if(this != &tt) 
		CopyFrom(tt);
	return *this;
}

TigerTree::TigerTree(const TigerTree& tt)
{
	CopyFrom(tt);
}

void TigerTree::ToBytes(ByteVector& v)
{
	for(size_t i = 0; i < tree_.size(); i++)
		if(tree_[i]) 
			std::copy((char*)tree_[i]->hash, (char*)tree_[i]->hash + TIGER_SIZE, std::back_inserter(v));
}

TigerTree::~TigerTree()
{
	Clear();
}


bool TigerTree::Empty() const
{
	return tree_.empty();
}

bool TigerTree::FromBytes(const ByteVector& v, file_offset_t size)
{
//	char debug[120];
//	sprintf(debug, "Enter function TigerTree::FromBytes %d", v.size());
//	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	if(v.size() % TIGER_SIZE)
		return false;
	
	uint height = 1;
	uint nodes = 1;
	file_offset_t pwr = 1;
	while(nodes < v.size() / TIGER_SIZE) {
		height ++;
		pwr *= 2;
		nodes += pwr;
	}
	Init(size, height);
	
	uint offset = 0, nd = 0;
	while(offset + TIGER_SIZE <= v.size() && nd < nodes) {
		if(tree_[nd]) {
			std::memcpy((char*)tree_[nd]->hash, &v[offset], TIGER_SIZE);
			offset += TIGER_SIZE;
		}
		++nd;
	}

	bool valid = Validate();
//	if(valid)
//		__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "TigerTree is valid.");
//	else
//		__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "TigerTree is not valid.");
	return Validate();	
}

bool TigerTree::Validate()
{
	if(tree_.empty())
		return false;

	bool valid = true;
	for(std::size_t parent = 0; valid && parent < tree_.size() / 2; ++parent)
	{
		if(tree_[parent] == INVALID_NODE)
			continue;

		const std::size_t left = parent * 2 + 1;
		const std::size_t right = parent * 2 + 2;

		if(left >= tree_.size() || tree_[left] == INVALID_NODE){
			valid = false;
//			if(!valid)__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "invalid node or wrong tree size");
		}
		else if(right >= tree_.size() || tree_[right] == INVALID_NODE){
			valid = std::memcmp(tree_[parent]->hash, tree_[left]->hash, TIGER_HASH_SIZE) == 0;
//			if(!valid)__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "right tree invalid");
		}
		else
		{
			assert(left < tree_.size() && right < tree_.size());
			assert(tree_[left] && tree_[right]);
			char tmp[1 + 2 * TIGER_HASH_SIZE];
			tmp[0] = 0x01;
			std::copy(tree_[left]->hash, tree_[left]->hash + TIGER_HASH_SIZE, tmp + 1);
			std::copy(tree_[right]->hash, tree_[right]->hash + TIGER_HASH_SIZE, tmp + 1 + TIGER_HASH_SIZE);

			char hash[TIGER_HASH_SIZE];
			Tiger(tmp, 1 + 2 * TIGER_HASH_SIZE, hash);
			valid = memcmp(tree_[parent]->hash, hash, TIGER_HASH_SIZE) == 0;
//			if(!valid)
//				__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "can't calculate the parent value.");
		}
	}

	if(!valid)
		Clear();
	return valid;

}

void TigerTree::Clear()
{
	for(uint i = 0; i < tree_.size(); i++) 
		if(tree_[i]) 
			delete tree_[i];
	
	tree_.clear();
}

Hashes::TTR TigerTree::Root() const
{
	if(Empty()) throw std::logic_error("Tree is empty");

	return Hashes::TTR(tree_[0]->hash);
}

} //namespace Hashes

/*int main()
{
	TigerTree tt;
	int f = open("Alash.jpg", O_RDONLY);
	int sz = lseek(f, 0, SEEK_END);
	std::cout << "file size: " << sz << std::endl;
	lseek(f, 0, SEEK_SET);
	tt.Create(sz, 4);
	int n;
	char buf[BLOCK_SIZE];
	int count = 0;
	//while(n = read(f, buf, BLOCK_SIZE)) {
	//	tt.AddBlock(buf, n, count);
//		count++;
//	}
	std::cout << "R: ";
//	PrintHex(tt.Root(), TIGER_SIZE, true);
//	file_offset_t x;
//	TigerTree tt;
//	do {
//		std::cin >> x;
//		tt.Create(x);
//		
//	}while(x);
	close(f);
	return 0;
}
*/

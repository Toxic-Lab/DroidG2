/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010  Andrey Stroganov <savthe@gmail.com>

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

#include "filemap.hpp"
#include <iostream>
#include <algorithm>

Integrity::Type FileMap::Status() const
{
	if(status_ == Integrity::NONE)
	{
		for(RangesContainer::const_iterator i = ranges_.begin(); i != ranges_.end(); ++i)
			status_ |= i->Status();
	}
	return status_;
}

Integrity::Type FileMap::Status(FileRange r) const
{
	Integrity::Type s = Integrity::NONE;
	for(RangesContainer::const_iterator i = ranges_.begin(); i != ranges_.end(); ++i)
		if(r.Intersects(*i))
			s |= i->Status();

	return s;
}

bool FileMap::IsRangeEmpty(FileRange r) const
{
	const Integrity::Type s = Status(r);
	return s == Integrity::EMPTY || s == Integrity::INVALID || s == (Integrity::EMPTY | Integrity::INVALID);
}

FileMap::FileMap()
{
	versize_ = 0;
	status_ = Integrity::NONE;
}

/*
bool FileMap::Validatable(const FilePart& part)
{
	uint64 firstByte = part.first_byte;
	const uint64 lastByte = part.last_byte;
	if(part.status == FilePart::FULL && versize_)
	{
		firstByte += firstByte % versize_ ? versize_ - firstByte % versize_ : 0;
		assert(firstByte % versize_ == 0);
		return (firstByte + versize_ - 1 <= lastByte) || (firstByte <= lastByte && lastByte == parts_.rbegin()->last_byte);
	}
	else
		return false;
}
*/

FileRange FileMap::GetForValidation()
{
	FileRange range;
	if(versize_ == 0) return FileRange();
	if(ranges_.empty()) return FileRange();
	
	for(RangesContainer::iterator i = ranges_.begin(); i != ranges_.end(); ++i)
		if(i->Status() == Integrity::FULL && i->Length() >= versize_)
		{	
			range.Offset(i->First() - i->First() % versize_);
			range.Length(versize_);
			if(range.Offset() < i->Offset()) 
				range.Offset(range.Offset() + versize_);
			if(range.Offset() + versize_ - 1 <= i->Last()) 
				return range;
			//if(result)
				//std::cout << "give: " << offset << "-" << offset + length - 1 << std::endl;
			//else
				//std::cout << "nothing";
		}
	FileRangeEx lastRange = *ranges_.rbegin();
	if(lastRange.Status() == Integrity::FULL && lastRange.Length() < versize_)
		return FileRange(lastRange);

	//std::cout << "nothing";
	return FileRange();
}

file_offset_t FileMap::CountBytes(Integrity::Type s) const
{
	file_offset_t count = 0;
	for(RangesContainer::const_iterator i = ranges_.begin(); i != ranges_.end(); ++i)
		if(i->Status() & s)
			count += i->Length();
	return count;
}

void FileMap::Print() const
{
	for(RangesContainer::const_iterator i = ranges_.begin(); i != ranges_.end(); ++i)
	{
		std::cout << "   " << i->First() << "-" << i->Last() << ":";
		switch(i->Status())
		{
			case Integrity::NONE: std::cout << "N"; break;
			case Integrity::VALID: std::cout << "V"; break;
			case Integrity::INVALID: std::cout << "I"; break;
			case Integrity::ACTIVE: std::cout << "A"; break;
			case Integrity::EMPTY: std::cout << "E"; break;
			case Integrity::FULL: std::cout << "F"; break;
		}
	}
	std::cout << std::endl;
}

FileRange FileMap::GetEmptyRange(file_offset_t max_length) const
{
	//std::cout << "GetEmptyPart: " << std::endl;
	//Show(parts_);
	assert(max_length);
	FileRange range;
	
	std::vector<FileRangeEx> empty_parts;
	//empty_parts.reserve(parts_.size());
	
	for(RangesContainer::const_iterator i = ranges_.begin(); i != ranges_.end(); ++i)
	{
		if(i->Status() == Integrity::EMPTY || i->Status() == Integrity::INVALID) 
		{
			if(versize_ && (i->First() % versize_ || (i->Last() + 1) % versize_))
			{
				empty_parts.clear();
				empty_parts.push_back(*i);
				break;
			}
			else
				empty_parts.push_back(*i);
		}
	}
	
	if(empty_parts.empty()) 
	{
		std::cout << "no empty parts" << std::endl;
		return FileRange();
	}
	
	FileRange r(empty_parts[rand() % empty_parts.size()]);
	
	//std::cout << "FileMap: vs: " << versize_ << " r: " << part.first_byte << "-" << part.last_byte;
	if(versize_ && r.First() % versize_)
		range = r;
	else if(versize_ && (r.Last() + 1) % versize_)
	{
		range.Offset(std::max(r.First(), r.Last() - r.Last() % versize_));
		range.Length(r.Last() - range.Offset() + 1);
	}
	else if(versize_ && r.Length() > versize_)
	{
		range.Offset(r.First() + versize_ * (rand() % (r.Length() / versize_)));
		range.Length(r.Last() - range.Offset() + 1);	
	}
	else
		range = r;
	
	//std::cout << "FOUND RANGE: " << range.First() << "-" << range.Last() << std::endl;
	range.Length(std::min(range.Length(), max_length));
	//std::cout << "NOW RANGE: " << range.First() << "-" << range.Last() << std::endl;
	range.Length(std::min(max_length - range.Offset() % max_length, range.Length()));
	//std::cout << "  giving: " << range.First() << "-" << range.Last() << std::endl;
	return range;
	//FilePartStatus tmp_st = RangeInfo(offset, offset + length - 1);
	//assert( tmp_st == FP_EMPTY || tmp_st  == FP_INVALID || tmp_st  == (FP_INVALID | FP_EMPTY));
	//SetRange(offset, offset + length - 1, FilePart::ACTIVE);
	
	//return true;
}

void FileMap::Init(file_offset_t size)
{
	ranges_.insert(FileRangeEx(FileRange(0, size), Integrity::EMPTY));
}

FileMap::RangesContainer::iterator FileMap::FindRange(file_offset_t offset)
{
	FileRangeEx p;
	p.Offset(offset);
	RangesContainer::iterator i = std::lower_bound(ranges_.begin(), ranges_.end(), p);  
	if(i == ranges_.end() || i->Offset() > offset)
		--i;
	return i;
}

void FileMap::ModifyRange(FileRange range, Integrity::Type st)
{
	if(range.Empty()) return;

	if(st & Integrity::EMPTY) assert(Status(range) == Integrity::ACTIVE);
	if(st & Integrity::FULL) assert(Status(range) == Integrity::ACTIVE);
	if(st & Integrity::VALID) assert(Status(range) == Integrity::FULL);
	if(st & Integrity::INVALID) assert(Status(range) == Integrity::FULL);
	if(st & Integrity::ACTIVE) 
		assert(Status(range) == Integrity::EMPTY || Status(range) == Integrity::INVALID || Status(range) == (Integrity::EMPTY | Integrity::INVALID));

	RangesContainer::iterator it_first = FindRange(range.First());
	RangesContainer::iterator it_last = FindRange(range.Last());	
	FileRangeEx first_part = *it_first;
	FileRangeEx last_part = *it_last;

	ranges_.erase(it_first, ++it_last);
	
	if(range.Offset() > first_part.Offset())
		ranges_.insert(FileRangeEx(first_part.Offset(), range.Offset() - first_part.Offset(), first_part.Status()));
	if(range.Last() < last_part.Last())
		ranges_.insert(FileRangeEx(range.Last() + 1, last_part.Last()- range.Last(), last_part.Status()));
	
	it_first = ranges_.insert(FileRangeEx(range, st)).first; 

	if(it_first != ranges_.begin())
		--it_first;

	std::size_t affected_ranges = 3;
	for(RangesContainer::iterator i = it_first; affected_ranges && i != ranges_.end(); --affected_ranges)
	{
		RangesContainer::iterator next = i;
		++next;
		if(next != ranges_.end() && i->Last() == next->First() - 1 && i->Status() == next->Status())
		{
			FileRangeEx part = *i;
			part.SetBoundary(part.First(), next->Last());
			ranges_.erase(i, ++next);
			std::pair<RangesContainer::iterator, bool> result = ranges_.insert(part);
			assert(result.second);
			i = result.first;
		}
		else
			++i;
	}
	status_ = Integrity::NONE;
}

void FileMap::FromStream(std::istream& s)
{
	ranges_.clear();

	file_offset_t offset = 0;
	file_offset_t first, last;
	uint status;

	while(s >> first >> last >> status)
	{
		//std::cout << "L: " << first << " " << last << " " << offset << std::endl;
		if(first != offset || last < first) 
			throw std::runtime_error("Bytes range hole");
		ranges_.insert(FileRangeEx(first, last - first + 1, status));
		offset = last + 1;
	}
}

void FileMap::ToStream(std::ostream& s) const
{
	for(RangesContainer::const_iterator i = ranges_.begin(); i != ranges_.end(); ++i)
	{
		Integrity::Type t = i->Status();
	       	if(t == Integrity::ACTIVE) t = Integrity::EMPTY;
		//std::cout << "S: " << i->First() << " " << i->Last() << std::endl;
		s << i->First() << " " << i->Last() << " " << uint(t) << " ";
	}
}


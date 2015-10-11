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

#ifndef FILEMAP_HPP
#define FILEMAP_HPP
#include "types.hpp"
#include <vector>
#include <set>
#include <assert.h>
#include <stdexcept>
#include <istream>
#include <ostream>
#include "filerange.hpp"

namespace Integrity {

typedef unsigned char Type;

enum {
	NONE = 0, 
	VALID = 1, 
	INVALID = 1 << 1, 
	ACTIVE = 1 << 2, 
	EMPTY = 1 << 3,
	FULL = 1 << 4,
	VALIDATABLE = 1 << 5
};

} //namespace Integrity

class FileRangeEx: public FileRange
{
public:
	FileRangeEx(const FileRange& r, Integrity::Type s = Integrity::NONE): FileRange(r), status_(s) {}
	FileRangeEx(file_offset_t offset, file_offset_t length, Integrity::Type s = Integrity::NONE): FileRange(offset, length), status_(s) {}
	FileRangeEx(): status_(Integrity::NONE) {}
	Integrity::Type Status() const { return status_; };
	void Status(Integrity::Type s) { status_ = s; }

protected:
	Integrity::Type status_;
};

class FileMap
{
public:
	FileMap();
	FileMap(file_offset_t size) { Init(size); }
	void Init(file_offset_t size);
	void SetVerificationSize(file_offset_t size) { versize_ = size; }
	
	void SetValid(FileRange r) { ModifyRange(r, Integrity::VALID); }
	void SetFull(FileRange r) { ModifyRange(r, Integrity::FULL); }
	void SetInvalid(FileRange r) { ModifyRange(r, Integrity::INVALID); }
	void SetActive(FileRange r) { ModifyRange(r, Integrity::ACTIVE); }
	void SetEmpty(FileRange r) { ModifyRange(r, Integrity::EMPTY); }

	bool IsRangeEmpty(FileRange) const;
	bool Empty() const { return ranges_.empty(); }
	FileRange GetEmptyRange(file_offset_t max = 128 * 1024) const;
	FileRange GetForValidation();

	template <typename AvailIter>
	FileRange GetEmptyRange(AvailIter avBegin, AvailIter avEnd, file_offset_t max = 128 * 1024) const
	{
		if(avBegin == avEnd) return GetEmptyRange(max);
		
		FileRange range;

		for(RangesContainer::const_iterator i = ranges_.begin(); i != ranges_.end(); ++i)
		{
			if(i->Status() != Integrity::EMPTY && i->Status() != Integrity::INVALID) continue;

			for(AvailIter avit = avBegin; avit != avEnd; ++avit)
			{
				if(avit->Intersects(*i)) 
				{
					//std::cout << "FileMap: ir: " << irange->first_byte << "-" 
					//<< irange->last_byte << "   i: " << i->first_byte << "-" << i->last_byte;

					const file_offset_t offset = std::max(avit->Offset(), i->Offset());
					assert(std::min(avit->Last(), i->Last()) >= offset);

					file_offset_t length = std::min(avit->Last(), i->Last()) - offset + 1;
					length = std::min(max, length);
					//length = std::min(max - offset % max, length);
					range.Set(offset, length);


					//range.Offset(std::max(avit->Offset(), i->Offset()));
					//range.Last(std::min(avit->Last(), i->Last()));
					///range.Length(std::min(max, range.Length()));
					///range.Length(std::min(max - range.Offset() % max, range.Length()));
					//std::cout << "   giving: " << offset << "-" << offset + length - 1 << std::endl;
					//FilePartStatus tmp_st = RangeInfo(offset, offset + length - 1);
					//assert( tmp_st == FP_EMPTY || tmp_st  == FP_INVALID || tmp_st  == (FP_INVALID | FP_EMPTY));
					//SetRange(offset, offset + length - 1, FilePart::ACTIVE);
					return range;
				}
			}
		}
		return FileRange();
	}

	template <typename Iter>
	void Dump(Iter iout, Integrity::Type s = 0xFF) const
	{
		for(RangesContainer::const_iterator i = ranges_.begin(); i != ranges_.end(); ++i)
			if(i->Status() & s)
				*iout++ = *i;
	}

	Integrity::Type Status() const;
	Integrity::Type Status(FileRange r) const;

	file_offset_t CountBytes(Integrity::Type s = 0xFF) const;
	void Print() const;

	void FromStream(std::istream& s);
	void ToStream(std::ostream& s) const;
	//std::size_t BytesSize() const { return parts_.size() * (sizeof(file_offset_t) * 2 + sizeof(FilePart::StatusType)); }
	file_offset_t FileSize() const { return ranges_.empty() ? 0 : ranges_.rbegin()->Last() + 1; }


private:
	typedef std::set <FileRangeEx> RangesContainer;

	void ModifyRange(FileRange r, Integrity::Type s);

	mutable Integrity::Type status_;
	RangesContainer::iterator FindRange(file_offset_t offset);
	RangesContainer ranges_;
	file_offset_t versize_;
};

#endif //FILEMAP_HPP


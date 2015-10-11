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

#ifndef RANGE_HPP
#define RANGE_HPP

#include <stdexcept>

template <class T> class Range
{
public:
	Range(): offset_(T()), length_(T()) {}
	Range(const T& off, const T& len): offset_(off), length_(len) {} 

	template <class Compatible> 
	Range(const Compatible& r): offset_(r.Offset()), length_(r.Length()) {}

	void Reset() { Set(T(), T()); }
	bool Empty() const { return length_ == T(); }

	void Set(const T& off, const T& len)
	{
		offset_ = off;
		length_ = len;
	}

	void SetBoundary(const T& first, const T& last)
	{
		if(first > last) throw std::range_error("first > last");

		offset_ = first;
		length_ = last - first + 1;
	}

	template <class Compatible>
	bool Intersects(const Range<Compatible>& r) const
	{
		return !(Offset() + Length() <= r.Offset() || Offset() >= r.Offset() + r.Length());
	}

	template <class Compatible>
	bool Includes(const Range<Compatible>& r) const
	{
		return First() <= r.First() && Last() >= r.Last();
	}

	bool Includes(const T& v)
	{
		return offset_ <= v && v < offset_ + length_;
	}

	T Offset() const { return offset_; }
	T Length() const { return length_; }
	T First() const { return offset_; }
	T Last() const { return offset_ + length_ - 1; }

	void Offset(const T& val) { offset_ = val; }
	void Length(const T& val) { length_ = val; }

private:
	T offset_;
	T length_;
};

template <class T1, class T2>
bool operator< (const Range<T1>& r1, const Range<T2>& r2)
{
	return r1.Offset() < r2.Offset();
}

template <class T1, class T2>
bool operator== (const Range<T1>& r1, const Range<T2>& r2)
{
	return r1.Offset() == r2.Offset() && r1.Length() == r2.Length();
}

template <class T1, class T2>
bool operator!= (const Range<T1>& r1, const Range<T2>& r2)
{
	return !(r1.Offset() == r2.Offset());
}

#endif //RANGE_HPP


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

#ifndef GUID_HPP
#define GUID_HPP

class GUID
{
public:
	typedef char* iterator;
	typedef const char* const_iterator;
	GUID() { Clear(); }
	GUID(const GUID& g) { CopyFrom(g); }
	GUID(const char*);
	GUID& operator= (const GUID&);
	void Generate();
	void CopyFrom(const GUID& g);
	bool Empty() const;
	void Clear();
	const char* Get() const { return bytes; }
	char* Bytes() { return bytes; }
	static unsigned int Size() { return 16; }
	iterator begin() { return bytes; }
	iterator end() { return bytes + 16; };
	const_iterator begin() const { return bytes; }
	const_iterator end() const { return bytes + 16; };

	template <typename T> 
	void Assign(T from) {
		for(int i = 0; i < 16; ++i) 
			bytes[i] = *from++;
	}
private:
	char bytes[16];
};

bool operator< (const GUID&, const GUID&);
bool operator== (const GUID&, const GUID&);
bool operator!= (const GUID&, const GUID&);

#endif //GUID_HPP

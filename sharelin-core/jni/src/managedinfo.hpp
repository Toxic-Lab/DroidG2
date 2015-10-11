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

#ifndef MANAGEDINFO_HPP
#define MANAGEDINFO_HPP

#include "sharedefs.hpp"
#include <boost/weak_ptr.hpp>
#include <stdexcept>

namespace Share {

class Manager;

class InvalidInfo: public std::runtime_error
{
public:
	InvalidInfo(): std::runtime_error("Invalid file info") {}
};

class ManagedInfo
{
public:
	ManagedInfo(FileInfoPtr pInfo, Manager* pManager): pInfo_(pInfo), pManager_(pManager) {}
	ManagedInfo(): pManager_(0) {};
	inline bool Lock() const { return !pInfo_.expired(); }

	ID Id() const { return Ptr()->id; }
	std::string Path() const { return Ptr()->path; }
	Hashes::SHA1 Sha1() const { return Ptr()->sha1; }
	Hashes::TTR Ttr() const { return Ptr()->ttr; }
	Hashes::ED2K Ed2k() const { return Ptr()->ed2k; }
	Hashes::MD5 Md5() const { return Ptr()->md5; }
	file_offset_t Size() const { return Ptr()->size; }
	uint Hits() const { return Ptr()->hits; }
	uint Requests() const { return Ptr()->requests; }
	ID Partial() const { return Ptr()->partial; }
	const Hashes::TigerTree& TigerTreeHash() const { return Ptr()->tigerTree; }
	const FileInfo& Info() const { return *Ptr(); }
	std::string Name() const 
	{
		if( Ptr()->name.empty() )
		{
			unsigned int pos = Ptr()->path.find_last_of("/");
			return Ptr()->path.substr(pos + 1);
		}

		return Ptr()->name;
	}

	void Sha1(const Hashes::SHA1& hash) 
	{ 
		if( !Ptr()->sha1.Empty() ) throw NotPermited("Sha1 is already set");
		Ptr()->sha1 = hash; 
		Recache(); 
	}

	void Ttr(const Hashes::TTR& hash) 
	{ 
		if( !Ptr()->ttr.Empty() ) throw NotPermited("Ttr is already set");
		Ptr()->ttr = hash; 
		Recache(); 
	}

	void Ed2k(const Hashes::ED2K& hash) 
	{ 
		if( !Ptr()->ed2k.Empty() ) throw NotPermited("Ed2k is already set");
		Ptr()->ed2k = hash; 
		Recache(); 
	}

	void Md5(const Hashes::MD5& hash) 
	{ 
		if( !Ptr()->md5.Empty() ) throw NotPermited("Md5 is already set");
		Ptr()->md5 = hash; 
		Recache(); 
	}

	void Name(const std::string& s)
	{
		Ptr()->name = s;
	}

	void Size(file_offset_t sz) 
	{ 
		if( Ptr()->size ) throw NotPermited("Size is already set");
		Ptr()->size = sz; 
		Recache(); 
	}
	
	void Partial(ID id) 
	{ 
		if( Ptr()->partial ) throw NotPermited("Partial ID is already set");
		Ptr()->partial = id; 
		Update(); 
	}

	void TigerTreeHash(const Hashes::TigerTree& tt) 
	{ 
		if( !Ptr()->tigerTree.Empty() ) throw NotPermited("Tiger tree is already set");

		if( Ptr()->ttr.Empty() ) Ptr()->ttr = tt.Root();
		else if ( Ptr()->ttr != tt.Root() ) throw NotPermited("Tiger tree root mismatch");

		Ptr()->tigerTree = tt; 
		Update(); 
	}

	void IncreaseHits() { Ptr()->hits++; Update(); }
	void IncreaseRequests() { Ptr()->requests++; Update(); }

protected:
	inline FileInfoPtr Ptr() const
	{
		if(!Lock()) throw InvalidInfo();
		return pInfo_.lock();
	}
	void Recache() {}
	void Update() {}

	boost::weak_ptr<FileInfo> pInfo_;
	Manager* pManager_;
};

} //namespace Share

#endif //MANAGEDINFO_HPP

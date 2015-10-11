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

#ifndef SHAREDEFS_HPP
#define SHAREDEFS_HPP

#include "hashes.hpp"
#include <string>
#include "types.hpp"
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include "tigertree.hpp"

namespace Share {

const int HASH_VALID_TIME = 3600 * 24 * 7;

namespace tags 
{
	struct name{};
	struct path{};
	struct hashed{};
	struct sha1{};
	struct md5{};
	struct ed2k{};
	struct ttr{};
	struct bp{};
	struct id{};
	struct ptr{};
}

enum {FILE_INVALID, FILE_REMOVED};

class EventsSubscriber
{
public:
	virtual void OnEvent(int event) = 0;
};

class NotFound: public std::runtime_error
{
public:
	NotFound(): std::runtime_error("Object not found") {}
};

class AlreadyShared: public std::runtime_error
{
public:
	AlreadyShared(): std::runtime_error("Object is already shared") {}
};

class NotPermited: public std::runtime_error
{
public:
	NotPermited(const std::string& msg): std::runtime_error(msg) {}
};

struct FileInfo
{
	FileInfo(): size(0), hashed(0), hits(0), requests(0), id(0), partial(INVALID_ID) {}

	std::string MakeName() const
	{
		if(!name.empty()) return name;
		const std::size_t i = path.find_last_of('/');
		if(i == std::string::npos) return path;
		return path.substr(i + 1);
	}
	
	std::string name;
	std::string path;
	Hashes::SHA1 sha1;
	Hashes::MD5 md5;
	Hashes::ED2K ed2k;
	Hashes::TTR ttr;
	Hashes::BP bp;
	file_offset_t size;
	time_t hashed;
	uint hits;
	uint requests;
	ID id;
	ID partial;
	Hashes::TigerTree tigerTree; 
};

typedef boost::shared_ptr<FileInfo> FileInfoPtr;

} //namespace Share

#endif //SHAREDEFS_HPP


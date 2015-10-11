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

#ifndef SEARCHDEFS_HPP
#define SEARCHDEFS_HPP

#include "media.hpp"
#include "types.hpp"
#include <string>
#include "hashes.hpp"
#include <set>
#include "sourcehost.hpp"
#include <ctime>

namespace Search {

const std::time_t BASE_RATE = 60 * 30;

struct Criteria
{
	Criteria(): media(0), min(0), max(0) {}
	std::string dn;
	Media::Type media;
	uint64 min;
	uint64 max;
	Hashes::SHA1 sha1;
	Hashes::TTR ttr;
	Hashes::MD5 md5;
	Hashes::ED2K ed2k;
	std::set <std::string> extensions;
};

struct Hit
{
	Hit(): available(false), partial(0), id(0), size(0), group(0){}

	bool available;
	uint32 partial;
	uint32 id;
	file_offset_t size;
	char group;
	Hashes::SHA1 sha1;
	Hashes::MD5 md5;
	Hashes::TTR ttr;
	Hashes::ED2K ed2k;
	std::string url;
	std::string dn;
	std::string meta;
	uint16 cachedSources;
	std::string comment;
};

struct Result
{
	Hit hit;
	SourceHost host;
	uint id;
};

} //namespace Search

#endif //SEARCHDEFS_HPP


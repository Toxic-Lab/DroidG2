/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009  Andrey Stroganov <savthe@gmail.com>

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

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <assert.h>

namespace FileSystem {

typedef unsigned char ElementType;
const ElementType REGULAR_FILE = 1;
const ElementType DIRECTORY = 1 << 1;
const ElementType SYMLINK = 1 << 2;

enum FollowType {FOLLOW, NOT_FOLLOW};

bool Exists(const std::string& path);
std::string Extension(const std::string& path);
std::string FileName(const std::string& path);
boost::intmax_t FileSize(const std::string& path);
std::string MakeAvailableName(const std::string& base);

void Rename(const std::string& path1, const std::string& path2);
void CreateDirectory(const std::string& path);
void Unlink(const std::string& path);
void CopyFile(const std::string& from, const std::string& to);
void PrepareFolder(const std::string&);

bool IsRegularFile(const std::string& path);
bool IsSymlink(const std::string&);
bool IsDirectory(const std::string&);

struct TypeChecker
{
	TypeChecker(ElementType mask): mask_(mask) {};

	bool operator()(const std::string& path)
	{
		if(IsSymlink(path) && !(mask_ & SYMLINK)) return false;

		return  ( (mask_ & REGULAR_FILE) && IsRegularFile(path) ) ||
			( (mask_ & DIRECTORY) && IsDirectory(path) );
	}	

	ElementType mask_;
};

template <typename OutIter, typename Predicate>
void ScanDirectory(const std::string& strPath, OutIter out, Predicate pred, FollowType follow)
{

	typedef boost::filesystem::directory_iterator iterator;

	if(!Exists(strPath)) return;

	boost::filesystem::path p(strPath);
	iterator end;
	iterator i;

	try { i = iterator(p); }
	catch(std::exception&) { return; }	
	
	for(; i != end; ++i)
	{
		if(boost::filesystem::is_symlink(i->path())) continue;

		const std::string s = i->string();
		if(pred(s))
			*out++ = s;
		if(follow == FOLLOW && boost::filesystem::is_directory(i->status()))
			ScanDirectory(s, out, pred, follow);
	}
}

template <typename OutIter>
void ScanDirectory(const std::string& strPath, OutIter out, ElementType mask, FollowType follow)
{
	ScanDirectory(strPath, out, TypeChecker(mask), follow);
}

} //namespace FileSystem

#endif //FILESYSTEM_HPP


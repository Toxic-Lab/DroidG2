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

#include "filesystem.hpp"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

namespace bfs = boost::filesystem;
using boost::lexical_cast;

namespace {

inline bfs::path Path(const std::string& path)
{
	return bfs::path(path);
}

}

namespace FileSystem {

std::string MakeAvailableName(const std::string& base)
{
	std::string name = base;
	for(int i = 1; i < 10000 && Exists(name); ++i)
		name = base + "_" + lexical_cast<std::string>(i);
	return name;
}

bool Exists(const std::string& path)
{
	return bfs::exists(Path(path));
}

void PrepareFolder(const std::string& path)
{
	if(!Exists(path))
		CreateDirectory(path);
}

void Rename(const std::string& path1, const std::string& path2)
{
	bfs::rename(Path(path1), Path(path2));
}

void Unlink(const std::string& path)
{
	bfs::remove_all(Path(path));
}

void CopyFile(const std::string& from, const std::string& to)
{
	bfs::copy_file(Path(from), Path(to));
}

boost::intmax_t FileSize(const std::string& path)
{
	return bfs::file_size(Path(path));
}

bool IsRegularFile(const std::string& path)
{
	return bfs::is_regular(Path(path));
}

bool IsDirectory(const std::string& path)
{
	return bfs::is_directory(Path(path));
}

bool IsSymlink(const std::string& path)
{
	return bfs::is_symlink(Path(path));
}

std::string Extension(const std::string& path)
{
	return Path(path).extension();
}

std::string FileName(const std::string& path)
{
	return Path(path).filename();
}

void CreateDirectory(const std::string& path)
{
	bfs::create_directory(Path(path));
}

} //namespace file_system

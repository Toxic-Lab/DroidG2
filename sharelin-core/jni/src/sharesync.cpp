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

#include "sharesync.hpp"
#include <algorithm>
#include "filesystem.hpp"

using FileSystem::IsRegularFile;
using FileSystem::FileSize;

namespace Share {
namespace Sync {

struct Predicate
{
	bool operator()(const std::string& path) const
	{
		return IsRegularFile(path) && FileSize(path) > 0;
	}
};

void Task::Run()
{
	Container found;
	for(Container::iterator i = scan_.begin(); i != scan_.end(); ++i)
	{
		try { FileSystem::ScanDirectory(*i, std::inserter(found, found.begin()), Predicate(), FileSystem::FOLLOW); }
		catch(std::exception& e)
		{
			result_.errors.push_back(Error(PATH_ERROR, *i, e.what()));
		}
	}

	Container inter;
	std::set_intersection(found.begin(), found.end(), validate_.begin(), validate_.end(), std::inserter(inter, inter.begin()));
	std::set_symmetric_difference(inter.begin(), inter.end(), validate_.begin(), validate_.end(), std::inserter(result_.removed, result_.removed.begin()));
	std::set_symmetric_difference(inter.begin(), inter.end(), found.begin(), found.end(), std::inserter(result_.created, result_.created.begin()));
	result_.found = found;
}

} //namespace Sync
} //namespace Share

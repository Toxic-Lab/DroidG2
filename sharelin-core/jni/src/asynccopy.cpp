/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2010  Andrey Stroganov <savthe@gmail.com>

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

#include "asynccopy.hpp"
#include "asyncfile.hpp"
#include "filesystem.hpp"
#include <stdexcept>
#include <iostream>

namespace async_copy {

void Task::Run()
{
	try 
	{ 
		FileSystem::CopyFile(from_, to_); 
		result_.status = GOOD;
		return;
	}
	catch ( std::exception& e )
	{
		result_.message = e.what();
	}
	result_.status = FILE_ERROR;
}

} //namespace async_copy

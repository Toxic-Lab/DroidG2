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

#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <map>
#include <string>
#include "sharedfile.hpp"
#include "fio.hpp"

class FileManager
{
public:
	SharedFile Open(const std::string& path, fio::Mode mode);
	void Transfer(const std::string& from, const std::string& to);
	void Close(AsyncFilePtr& pFile);

private:
	typedef std::map <std::string, AsyncFilePtr> Pointers;
	Pointers pointers_;
};

#endif //FILEMANAGER_HPP



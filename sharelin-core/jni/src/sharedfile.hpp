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

#ifndef SHAREDFILE_HPP
#define SHAREDFILE_HPP

#include <boost/shared_ptr.hpp>
#include "types.hpp"
#include "asyncfile.hpp"
#include <stdexcept>

class FileManager;

typedef boost::shared_ptr <AsyncFile> AsyncFilePtr;

class SharedFile
{
public:
	class FileClosed: public std::runtime_error
	{
	public:
		FileClosed(): std::runtime_error("File is pointer") {}
	};

	SharedFile(): pManager_(0) {}
	SharedFile(FileManager*, AsyncFilePtr pFile);
	~SharedFile();
	void Read(char* buffer, int64 length, file_offset_t offset);
	void Write(const char* buffer, int64 length, file_offset_t);	
	void Close();
	bool IsClosed() const { return !pFile_; }
	void Sync() { Get()->Sync(); }


private:
	AsyncFilePtr Get();

	FileManager* pManager_;
	AsyncFilePtr pFile_;
};

#endif //SHAREDFILE_HPP


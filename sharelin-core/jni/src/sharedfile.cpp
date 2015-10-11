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

#include "sharedfile.hpp"
#include "filemanager.hpp"
#include <assert.h>

SharedFile::SharedFile(FileManager* pMan, AsyncFilePtr pFile):
	pManager_(pMan), pFile_(pFile)
{
	assert(pMan);
}

void SharedFile::Read(char* buffer, int64 length, file_offset_t offset)
{
	Get()->Read(buffer, length, offset);	
}

void SharedFile::Write(const char* buffer, int64 length, file_offset_t offset)
{
	Get()->Write(buffer, length, offset);
}

AsyncFilePtr SharedFile::Get()
{
	if(IsClosed()) throw FileClosed();
	return pFile_;
}

void SharedFile::Close()
{
	if(!IsClosed())
	{
		assert(pManager_);
		pManager_->Close(pFile_);
	}
}

SharedFile::~SharedFile()
{
	Close();
}

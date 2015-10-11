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

#include "asyncfile.hpp"
#include <boost/static_assert.hpp>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <iostream>
#include <assert.h>
#include <android/log.h>

void AsyncFile::Open(const std::string& fileName, fio::Mode mode)
{
	BOOST_STATIC_ASSERT(sizeof(off_t) >= 4);

	int flags = 0;

	if(Opened()) 
		throw AlreadyOpened();

	switch (mode)
	{
		case fio::read: flags = O_RDONLY; break;
		case fio::write: flags = O_WRONLY | O_CREAT; break;
		case fio::rw: flags = O_RDWR | O_CREAT; break;
		default: throw BadMode();
	}

	fd_ = open(fileName.c_str(), flags);

	if(fd_ != -1)
	{
		mode_ = mode;
		name_ = fileName;
	}
	else throw Error(errno);
}

void AsyncFile::Sync()
{
	fsync(fd_);
}

void AsyncFile::Read(char* buffer, int64 length, int64 offset)
{
	off_t length2 = static_cast<off_t>(length);
	off_t offset2 = static_cast<off_t>(offset);

	if(pread(fd_, static_cast <void*> (buffer), length2, offset2) != length2)
		throw Error(errno);
}

void AsyncFile::Write(const char* buffer, int64 length, int64 offset)
{
	off_t len = static_cast<off_t>(length);
	off_t offset2 = static_cast<off_t>(offset);

	char* temp = new char[len];
	for(int i=0; i<len; ++i)
		temp[i] = buffer[i];

	if(pwrite(fd_, static_cast <void*> (temp), len, offset2) != len){
		throw Error(errno);
	}
	delete[] temp;
}

void AsyncFile::Close()
{
	if(Opened())
	{
		close(fd_);
		mode_ = fio::none;
	}
}

bool AsyncFile::Opened() const
{
	return mode_ != fio::none;
}


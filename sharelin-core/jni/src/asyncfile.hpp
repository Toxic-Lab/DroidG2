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

#ifndef ASYNCFILE_HPP
#define ASYNCFILE_HPP

#include "types.hpp"
#include <string>
#include <stdexcept>
#include "fio.hpp"
#include <boost/system/error_code.hpp>

class AsyncFile
{
public:
	class Error: public std::exception
	{
	public:
		Error(int n): e(n, boost::system::system_category()) {}
		int value() const { return e.value(); }
		const char* what() const throw () { return e.message().c_str(); }
	private:
		boost::system::error_code e;
	};

	class AlreadyOpened: public std::runtime_error
	{
	public:
		AlreadyOpened(): std::runtime_error("File is already opened") {}
	};

	class BadMode: public std::runtime_error
	{
	public:
		BadMode(): std::runtime_error("Bad open mode") {}
	};

	AsyncFile(): mode_(fio::none) {}
	void Open(const std::string& fileName, fio::Mode mode);
	void Close();
	void Read(char* buffer, int64 length, int64 offset);
	void Write(const char* buffer, int64 length, int64 offset);
	bool Opened() const;
	fio::Mode GetMode() const { return mode_; }
	std::string PathName() const { return name_; }
	void Sync();

private:
	int fd_;
	std::string name_;
	fio::Mode mode_;
};

#endif //ASYNCFILE_HPP


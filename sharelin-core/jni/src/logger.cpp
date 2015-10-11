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

#include "logger.hpp"
#include <string>
#include <ctime>
#include "filesystem.hpp"

using FileSystem::CopyFile;
using FileSystem::Exists;
using FileSystem::Unlink;

namespace {

template <typename T>
T& Timestamp(T& out)
{
	std::time_t rawtime;
  	struct tm * timeinfo;
  	char buffer [80];

  	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );

  	strftime (buffer, 80, "[%H:%M:%S]", timeinfo);
	out << buffer;
	return out;
}

} //namespace 

Logger& operator<<(Logger& log, std::ostream& (*f)(std::ostream&))
{
	if(log.level_ <= log.verb_) f(std::cout);
	if(log.file_.is_open()) f(log.file_);
	log.Update();

	return log;
}

void Logger::Update()
{
	if(file_.is_open() && file_.tellg() > maxSize_)
	{
		file_.close();
		const std::string bak = name_ + ".bak";
		if(Exists(bak)) Unlink(bak); 
		CopyFile(name_, name_ + ".bak");
	}

	if(!file_.is_open() && maxSize_ > 0 && !name_.empty())
		Open(name_, maxSize_);
}

Logger& Logger::Log(unsigned int level)
{
	level_ = level;

	file_ << level_ << " ";
	Timestamp(file_);
	if(level_ <= verb_) Timestamp(std::cout);

	*this << " ";

	return *this;
}

bool Logger::Open(const std::string& fname, unsigned int max)
{
	name_ = fname;
	maxSize_ = max;
	file_.open(fname.c_str(), std::fstream::out);
	return file_.good();
}

void Logger::Close()
{
	if(file_.is_open()) file_.close();
}

bool Logger::IsOpen() const 
{
	return file_.is_open();
}

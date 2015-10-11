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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <iostream>
#include <ostream>
#include <ctime>
#include <string>

class Logger
{
public:
	Logger(): level_(0), verb_(0) {}
	Logger(unsigned int n): level_(0), verb_(n) {}

	bool Open(const std::string&, unsigned int);
	bool IsOpen() const;
	void Close();
	void VerboseLevel(unsigned int n) { verb_ = n; }
	void MaxSize(unsigned int sz) { maxSize_ = sz; }
	Logger& Log(unsigned int level = 0);
	void Update();

	template <typename T> friend Logger& operator<<(Logger&, const T&);
	friend Logger& operator<<(Logger& log, std::ostream& (*fn)(std::ostream&));

private:
	unsigned int maxSize_;
	unsigned int level_;
	unsigned int verb_;
	std::fstream file_;
	std::time_t tsFile_;
	std::string name_;
};

Logger& operator<<(Logger& log, std::ostream& (*f)(std::ostream&));

template<typename T>
Logger& operator<<(Logger& log, const T& data)
{
	if(log.level_ <= log.verb_) std::cout << data; 
	if(log.file_.is_open()) log.file_ << data;

	return log;
}

#endif //LOGGER_HPP


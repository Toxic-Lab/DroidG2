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

#ifndef ASYNCCOPY_HPP
#define ASYNCCOPY_HPP

#include "asynctask.hpp"
#include "types.hpp"
#include "sharedfile.hpp"
#include <boost/function.hpp>
#include <string>

namespace async_copy {

enum {ERROR, GOOD, FILE_ERROR};

struct Result: public async_task::Result
{
	std::string from;
	std::string to;
};

class Task: public async_task::Task
{
public:
	typedef boost::function< void(Result) > Handler;
	Task(const std::string& from, const std::string& to, Handler h):
		from_(from), to_(to), handler_ (h) 
	{
		result_.from = from;
		result_.to = to;
	}

protected:
	void Run();
	void Complete() { handler_(result_); }

	std::string from_;
	std::string to_;
	Handler handler_;
	Result result_;
};

} //namespace async_copy

#endif //ASYNCCOPY_HPP


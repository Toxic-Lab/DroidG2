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

#ifndef ASYNCFUNCTION_HPP
#define ASYNCFUNCTION_HPP

#include "asynctask.hpp"
#include "types.hpp"
#include "sharedfile.hpp"
#include <boost/function.hpp>
#include <string>

namespace async_function {

typedef boost::function< int() > Worker;
typedef boost::function< void(int) > Handler;

class Task: public async_task::Task
{
public:
	Task(Worker w, Handler h):
		worker_(w), handler_ (h) 
	{ }

protected:
	void Run() { result_ = worker_(); }
	void Complete() { handler_(result_); }

	Worker worker_;
	Handler handler_;
	int result_;
};

} //namespace async_function

#endif //ASYNCFUNCTION_HPP


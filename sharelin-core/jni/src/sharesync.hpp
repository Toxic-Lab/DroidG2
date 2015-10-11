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

#ifndef SHARESYNC_HPP
#define SHARESYNC_HPP

#include "asynctask.hpp"

#include <boost/function.hpp>

#include <string>
#include <set>
#include <vector>

namespace Share {
namespace Sync {

typedef std::set<std::string> Container;	

enum {PATH_ERROR};

struct Error
{
	Error(): id(0) {}
	Error(int id_, const std::string& path_, const std::string& message_): id(id_), path(path_), message(message_) {}

	int id;
	std::string path;
	std::string message;
};

struct Result: public async_task::Result
{
	Container created;
	Container removed;
	Container found;
	std::vector<Error> errors;
};

class Task: public async_task::Task
{
public:
	typedef boost::function< void(Result) > Handler;

	Task(Handler h): handler_ (h) {}
	void Scan(const std::string& s) { scan_.insert(s); }
	void Validate(const std::string& s) { validate_.insert(s); }

protected:
	void Run();
	void Complete() { handler_(result_); }

	Handler handler_;
	Container scan_;
	Container validate_;
	Result result_;
};

} //namespace Sync
} //namespace Share

#endif //SHARESYNC_HPP


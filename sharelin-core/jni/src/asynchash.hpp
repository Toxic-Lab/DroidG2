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

#ifndef ASYNCHASH_HPP
#define ASYNCHASH_HPP

#include "asynctask.hpp"
#include "hashes.hpp"
#include "types.hpp"
#include "filerange.hpp"
#include <boost/function.hpp>
#include <string>

namespace async_hash {

enum {GOOD, FILE_ERROR};

struct Result: public async_task::Result
{
	Hashes::SHA1 sha1;
	Hashes::TTR ttr;
	Hashes::MD5 md5;
	Hashes::ED2K ed2k;
	FileRange range;
	std::string path;
	uint rate;
};

class Task: public async_task::Task
{
public:
	typedef boost::function< void(Result) > Handler;
	Task(const std::string& path, Hashes::Mask::Type m, FileRange r, Handler h, uint rate = 0):
		path_(path), mask_(m), range_(r), handler_ (h), rate_(rate) 
	{
		result_.range = r;
		result_.path = path;
	}
protected:
	void Run();
	void Complete() { handler_(result_); }

	std::string path_;
	Hashes::Mask::Type mask_;
	FileRange range_;
	Handler handler_;
	Result result_;
	uint rate_; //bps
};

} //namespace async_hash

#endif //ASYNCHASH_HPP


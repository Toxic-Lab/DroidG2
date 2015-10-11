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

#ifndef ASYNCTASK_HPP
#define ASYNCTASK_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>
#include <stdexcept>

namespace async_task {

class Server;

class WorkerIsBusy: public std::exception
{
public:
	WorkerIsBusy(): std::exception() {}
};

class ServerNotConnected: public std::exception
{
public:
	ServerNotConnected(): std::exception() {}
};

struct Result
{
	Result(): status(0) {}
	unsigned int status;
	std::string message;
};

class Task
{
public:
	typedef boost::shared_ptr <Task> Ptr;
	typedef boost::weak_ptr <Task> WeakPtr;

	Task(): aborted_(false) {}
	virtual ~Task() {} 

	bool Aborted() { return aborted_; }
	void Abort() { aborted_ = true; }

	virtual void Run() = 0;
	virtual void Complete() = 0;

private:
	bool aborted_;
};

class Work 
{
public:
	Work(): pServer_(0) {}
	Work(Server* pServer): pServer_(pServer) {}
	void Attach(Server*);
	bool Busy();
	void Abort();

	template<typename T>
	void Run(const T& t)
	{
		if(Busy()) throw WorkerIsBusy();
		Task::Ptr p(new T(t));
		Start(p);
	}

private:
	void Start(Task::Ptr);
	Task::WeakPtr pTask_;
	Server* pServer_;
};

} // namespace async_task

#endif //ASYNCTASK_HPP


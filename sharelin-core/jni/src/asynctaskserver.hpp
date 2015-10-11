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

#ifndef ASYNCTASKSERVER_HPP
#define ASYNCTASKSERVER_HPP

#include <list>
#include <map>
#include "asynctask.hpp"
#include "asyncfunction.hpp"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

namespace async_task {

class Server: public boost::noncopyable
{
public:
	Server(boost::asio::io_service& io, unsigned int sz): io_(io), size_(sz) {} 

	void Add(Task::Ptr);
	void Abort(Task::Ptr); 
	bool Has(Task::Ptr);
	void AbortAll();

	//Handle Add(async_function::Worker w, async_function::Handler h)
	//{
	//	return Add(async_function::Task(w, h));
	//}


private:
	struct Job
	{
		Job(Task::Ptr p, Server* pServ): 
			pTask(p), pServer(pServ) {} 

		void operator()()
		{
			pTask->Run();
			pServer->Done(pTask);
		}

		Task::Ptr pTask;
		Server* pServer;
	};

	typedef boost::shared_ptr<boost::thread> ThreadPtr;
	void Update();
	void Done(Task::Ptr);
	void Complete(Task::Ptr);

	boost::asio::io_service& io_;
	std::list <Task::Ptr> tasks_;
	std::map<Task::Ptr, ThreadPtr> threads_;
	unsigned int size_;
	
};

} //namespace async_task


#endif //ASYNCTASKSERVHER_HPP


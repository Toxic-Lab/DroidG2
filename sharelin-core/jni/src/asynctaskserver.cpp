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

#include "asynctaskserver.hpp"
#include <assert.h>
#include "io.hpp"
#include <iostream>

namespace async_task {

void Server::Add(Task::Ptr p)
{
	tasks_.push_back(p);
	Update();
}

void Server::Update()
{
	if(tasks_.empty()) return;
	if(threads_.size() == size_) return;

	assert(threads_.size() < size_);

	Task::Ptr p = tasks_.front();
	tasks_.pop_front();
	assert(!p->Aborted());

	threads_[p] = ThreadPtr(new boost::thread(Job(p, this)));
}

void Server::AbortAll()
{
	tasks_.clear();
	for(std::map<Task::Ptr, ThreadPtr>::iterator i = threads_.begin(); i != threads_.end(); ++i)
		Abort(i->first);
}

void Server::Abort(Task::Ptr p)
{
	if(threads_.find(p) != threads_.end())
	{
		p->Abort();
		threads_[p]->join();
	}
	else
	{
		for(std::list<Task::Ptr>::iterator i = tasks_.begin(); i != tasks_.end(); ++i)
			if(*i == p)
			{
				tasks_.erase(i);
				break;
			}
	}
}

void Server::Done(Task::Ptr p)
{
	io_.post(boost::bind(&Server::Complete, this, p));
}

bool Server::Has(Task::Ptr p)
{
	return threads_.find(p) != threads_.end() || std::find(tasks_.begin(), tasks_.end(), p) != tasks_.end();
}

void Server::Complete(Task::Ptr p)
{
	assert(threads_.find(p) != threads_.end());
	threads_[p]->join();
	threads_.erase(p); //BEFORE Complete() so now work is not busy
	if(!p->Aborted()) p->Complete();
	Update();
}

} //namespace async_task


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

#include "asynctask.hpp"
#include "asynctaskserver.hpp"

namespace async_task {

bool Work::Busy()
{
	if(!pServer_) throw ServerNotConnected();
	return pServer_->Has(pTask_.lock());
}

void Work::Start(Task::Ptr p)
{
	if(!pServer_) throw ServerNotConnected();
	pTask_ = p;
	pServer_->Add(p);
}

void Work::Abort()
{
	if(!pServer_) throw ServerNotConnected();
	pServer_->Abort(pTask_.lock());
}

void Work::Attach(Server* pServ)
{
	if(pServer_ && Busy()) throw WorkerIsBusy();
	pServer_ = pServ;
}

} //namespace async_task

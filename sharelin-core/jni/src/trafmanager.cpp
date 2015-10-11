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

#include "trafmanager.hpp"

TrafManager::TrafManager(uint maxShare):
	maxShare_(maxShare),
	share_(0),
	now_(0),
	usage0_(0),
	usage1_(0),
	usage2_(0),
	threshold_(false)
{
}

void TrafManager::OnTimer(std::time_t now)
{
	if(now_ == now) return;

	now_ = now;
	threshold_ = share_ == 0;
	share_ = maxShare_;

	usage2_ = usage1_;
	usage1_ = usage0_;
	usage0_ = 0;

	Sweep();

	if(clients_.empty()) return;

	const uint reserved = share_ / 10;
	share_ -= reserved;

	const uint minQuota = reserved / clients_.size();

	for(Container::iterator i = clients_.begin(); i != clients_.end(); ++i)
		i->second.amount = minQuota;
}

uint TrafManager::Usage() const
{
	return (usage0_ + usage1_ + usage2_) / 3;
}

uint TrafManager::Use(Ip::Endpoint endpoint, uint want)
{
	Quota& q = clients_[endpoint];
	q.timestamp = now_;
	
	const uint useShare = std::min(share_, want);
	share_ -= useShare;
	q.amount += useShare;
	const uint use = std::min(want, q.amount);
	q.amount -= use;

	usage0_ += use;

	return use;
}

void TrafManager::Sweep()
{
	for(Container::iterator i = clients_.begin(); i != clients_.end();)
	{
		if(now_ - i->second.timestamp > 3)
		{
			Container::iterator next = i;
			++next;
			clients_.erase(i);
			i = next;
		}
		else ++i;
	}
}


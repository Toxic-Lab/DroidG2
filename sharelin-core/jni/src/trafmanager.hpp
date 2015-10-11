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

#ifndef TRAFMANAGER_HPP
#define TRAFMANAGER_HPP

#include <map>
#include "ip.hpp"
#include <ctime>
#include "types.hpp"

class TrafManager
{
public:
	TrafManager(uint maxShare = 0);
	void OnTimer(std::time_t);
	uint Use(Ip::Endpoint, uint);
	void SetShare(uint max) { maxShare_ = max; }
	uint Usage() const; 
	uint Amount() const { return share_; }
	bool ThresholdReached() const { return threshold_; }
	uint CountClients() const { return clients_.size(); }

private:
	struct Quota
	{
		Quota(): amount(0), timestamp(0) {}
		Quota(uint n, std::time_t t): amount(n), timestamp(t) {}
		uint amount;
		std::time_t timestamp;
	};
	typedef std::map<Ip::Endpoint, Quota> Container;

	void Sweep();

	uint maxShare_;
	uint share_;
	std::time_t now_;
	uint usage0_;
	uint usage1_;
	uint usage2_;
	Container clients_;
	bool threshold_;
};

#endif //TRAFMANAGER_HPP


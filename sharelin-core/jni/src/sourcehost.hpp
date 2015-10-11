/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010 Andrey Stroganov <savthe@gmail.com>

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

#ifndef SOURCEHOST_HPP
#define SOURCEHOST_HPP

#include "types.hpp"
#include "ip.hpp"
#include "guid.hpp"
#include <vector>
#include <boost/shared_ptr.hpp>
#include <ctime>

struct SourceHost
{
	typedef std::vector<Ip::Endpoint> HubsContainer;
	bool FromUri(std::string);

	Ip::Endpoint endpoint;
	std::string uri;
	GUID guid;
	HubsContainer hubs;
	std::string nick;
};

struct SourceHostEx: public SourceHost
{
	typedef boost::shared_ptr<SourceHostEx> Ptr;

	SourceHostEx(): connectAfter(0), connections(0), maxConnections(1) {}
	SourceHostEx(const SourceHost& src): SourceHost(src), connectAfter(0), connections(0), maxConnections(1) {}

	std::time_t connectAfter;
	uint connections;
	uint maxConnections;
	uint32 vendor;
};

typedef std::vector<SourceHost> SourceHostsContainer;
typedef boost::shared_ptr<SourceHost> SourceHostPtr;

#endif //SOURCEHOST_HPP


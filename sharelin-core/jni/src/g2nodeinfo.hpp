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

#ifndef G2NODEINFO_HPP
#define G2NODEINFO_HPP

#include "ip.hpp"
#include "guid.hpp"
#include "types.hpp"
#include <ctime>
#include "g2defs.hpp"

namespace G2 {

struct HubInfo 
{
	HubInfo(): key_(0), seen_(0), tsKey_(0), searchAfter_(0) {}
	HubInfo(Ip::Endpoint ep): endpoint_(ep), key_(0), seen_(0), tsKey_(0), searchAfter_(0) {}
	HubInfo(Ip::Endpoint ep, std::time_t t): endpoint_(ep), key_(0), seen_(t), tsKey_(0), searchAfter_(0) {}

	void Clear()
	{
		endpoint_.Clear();
		proxy_.Clear();
		key_ = 0;
		seen_ = 0;
		tsKey_ = 0;
		searchAfter_ = 0;
	}

	void SetEndpoint(Ip::Endpoint endpoint)
	{
		if(endpoint_ != endpoint)
		{
			Clear();
			endpoint_ = endpoint;
		}
	}

	void QueryKey(uint32 k, std::time_t now)
	{
		key_ = k;
		tsKey_ = now;
	}

	void Proxy(Ip::Endpoint proxy)
	{
		key_ = 0;
		tsKey_ = 0;
		proxy_ = proxy;
	}

	bool IsKeyValid(std::time_t now) const
	{
		return key_ && now - tsKey_ <= G2::QKEY_LIFE_TIME;
	}

	void DiscardKey()
	{
		key_ = 0;
	}

	Ip::Endpoint GetEndpoint() const { return endpoint_; }
	Ip::Address GetAddress() const { return endpoint_.address; }
	uint16 GetPort() const { return endpoint_.port; }

	Ip::Endpoint Proxy() const { return proxy_; }
	void DiscardProxy() { Proxy(Ip::Endpoint()); }

	bool Empty() const { return endpoint_.Empty(); }

	void SearchAfter(std::time_t t) { searchAfter_ = t; }
	std::time_t SearchAfter() const { return searchAfter_; }

	uint32 QueryKey() const { return key_; }

	std::time_t KeyIssued() const { return tsKey_; }
	void KeyIssued(std::time_t t) 
	{ 
		key_ = 0;
		tsKey_ = t; 
	}

	void Seen(std::time_t t) { seen_ = t; }
	std::time_t Seen() const { return seen_; }




private:
	Ip::Endpoint endpoint_;
	Ip::Endpoint proxy_;
	uint32 key_;
	std::time_t seen_;
	std::time_t tsKey_;
	std::time_t searchAfter_;
};

struct NodeInfo
{
	Ip::Endpoint endpoint;
	GUID guid;
	uint32 vendor;
};

} //namespace G2

bool operator< (const G2::HubInfo& hub1, const G2::HubInfo& hub2);
bool operator== (const G2::HubInfo& hub1, const G2::HubInfo& hub2);
bool operator!= (const G2::HubInfo& hub1, const G2::HubInfo& hub2);

#endif //G2NODEINFO_HPP

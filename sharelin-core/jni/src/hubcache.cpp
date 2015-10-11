/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2009  Sharelin Project

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

#include <fstream>
#include "hubcache.hpp"
#include <stdexcept>
#include <string>
#include <sstream>
#include <stdexcept>
#include <ctime>
#include <cassert>
#include <iostream>

namespace {

const std::size_t LINEAR_MAX = 300;

}

HubCache::HubCache(unsigned int max):
	maxHubs_(max)
{
}

void HubCache::SetCapacity(unsigned int max)
{
	maxHubs_ = max;
	Purge();
}

void HubCache::Save(const std::string& fname)
{
	std::ofstream f(fname.c_str());
	if(!f.good()) throw std::runtime_error("Couldn't open file");

	f << "#Sharelin's hub cache." << std::endl;
	f << "#To add hub manually just type it's address and port. Don't add timestamp." << std::endl;
	for(Cache::iterator i = cache_.begin(); i != cache_.end(); ++i)
		f << i->Info().GetEndpoint() << " " << i->Info().Seen() << "\n";
}

void HubCache::Load(const std::string& fname)
{
	std::ifstream f(fname.c_str());
	if(!f.good()) throw std::runtime_error("Couldn't open file");

	std::string line;
	while(std::getline(f, line))
	{
		if(line.empty() || line[0] == '#')
			continue;

		std::stringstream s(line);
		std::string strEndpoint;
		uint32 ts = 0;
		s >> strEndpoint >> ts;	
		Ip::Endpoint ep;
		if(ep.FromString(strEndpoint)) 
			Touch(ep, ts);
	}
	f.close();
}

void HubCache::Touch(Ip::Endpoint endpoint, std::time_t ts)
{
	if(endpoint.Empty()) throw std::runtime_error("Hub endpoint is empty");

	if(ts == 0 || ts > std::time(0)) ts = std::time(0);

	Cache::index_iterator<tagAddress>::type i = cache_.get<tagAddress>().find(endpoint.address);
	if(i == cache_.get<tagAddress>().end())
		Update(G2::HubInfo(endpoint, ts));
	else
	{
		i->ptr->Seen(ts);
		Recache(i->ptr);
	}
}

void HubCache::Update(const G2::HubInfo& hub)
{
	if(hub.Empty()) throw std::runtime_error("Hub endpoint is empty");

	Cache::index_iterator<tagAddress>::type i = cache_.get<tagAddress>().find(hub.GetAddress());
	if(i == cache_.get<tagAddress>().end())
	{
		HubInfoPtr p(new G2::HubInfo(hub));
		cache_.insert(CachedHub(p));
	}
	else
	{
		*(i->ptr) = hub;
		Recache(i->ptr);
	}

	Purge();
}

void HubCache::Recache(HubCache::HubInfoPtr p)
{
	Cache::index_iterator<tagAddress>::type i = cache_.get<tagAddress>().find(p->GetAddress());
	if(i == cache_.get<tagAddress>().end())
		cache_.insert(CachedHub(p));
	else
	{
		cache_.replace(i, CachedHub(p));
	}
}

std::size_t HubCache::Size() const
{
	return cache_.size();
}

void HubCache::Purge()
{
	const std::time_t now = std::time(0);

	while(true)
	{
		const bool purge = 
			cache_.size() > maxHubs_ ||
			(!cache_.empty() && now - cache_.get<tagSeen>().rbegin()->seen > G2::HUB_LIFE_TIME);

		if(purge) cache_.get<tagSeen>().erase(cache_.get<tagSeen>().rbegin()->seen);
		else break;
	}
}

G2::HubInfo HubCache::GetHubWithoutKey() const
{
	const std::time_t now = std::time(0);

	uint count = 0;
	uint linear_count = LINEAR_MAX;
	Cache::index_iterator<tagSeen>::type i;
	for(i = cache_.get<tagSeen>().begin(); i != cache_.get<tagSeen>().end() && linear_count--; ++i)
		if(!i->Info().IsKeyValid(now) && now - i->Info().KeyIssued() > G2::QKEY_QUERY_TIMEOUT) ++count;

	if(count == 0) return G2::HubInfo();

	uint n = 1 + rand() % count;
	i = cache_.get<tagSeen>().begin();
	while(n > 0 && i != cache_.get<tagSeen>().end())
	{
		assert(i != cache_.get<tagSeen>().end());
		if(!i->Info().IsKeyValid(now) && now - i->Info().KeyIssued() > G2::QKEY_QUERY_TIMEOUT) 
			--n;
		if(n == 0) break;
		++i;
	}

	assert(i != cache_.get<tagSeen>().end());
	assert(!i->Info().IsKeyValid(now));

	return i->Info();
}

void HubCache::Remove(Ip::Address addr)
{
	cache_.get<tagAddress>().erase(addr);
}

void HubCache::DiscardProxy(Ip::Address addr)
{
	for(Cache::iterator i = cache_.begin(); i != cache_.end(); ++i)
	{
		HubInfoPtr p = i->ptr;
		if(p->Proxy().address == addr)
		{
			p->DiscardProxy();
			Recache(p);
		}
	}
}

G2::HubInfo HubCache::Find(Ip::Address addr) const
{
	G2::HubInfo hub;

	Cache::index_iterator<tagAddress>::type i = cache_.get<tagAddress>().find(addr);
	if(i != cache_.get<tagAddress>().end()) hub = i->Info();

	return hub;
}

G2::HubInfo HubCache::GetRandomHub() const
{
	if(cache_.empty()) return G2::HubInfo();

	const uint max = cache_.size() > LINEAR_MAX ? LINEAR_MAX : cache_.size();
	Cache::index_iterator<tagSeen>::type i = cache_.get<tagSeen>().begin();
	for(uint n = rand() % max; n > 0; --n, ++i);

	return i->Info();
}


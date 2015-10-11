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

#ifndef HUBCACHE_HPP
#define HUBCACHE_HPP

#include <ctime>
#include <cassert>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/shared_ptr.hpp>

#include "types.hpp"
#include "ip.hpp"
#include "g2nodeinfo.hpp"
#include "g2defs.hpp"

class HubCache 
{
private:
	typedef boost::shared_ptr<G2::HubInfo> HubInfoPtr;

	struct CachedHub
	{
		CachedHub(HubInfoPtr p): address(p->GetAddress()), seen(p->Seen()), key(p->QueryKey()), ptr(p) {}

		inline const G2::HubInfo& Info() const { return *ptr; }

		Ip::Address address;
		std::time_t seen;
		uint32 key;
		HubInfoPtr ptr;
	};

	struct tagAddress{};
	struct tagSeen{};
	struct tagKey{};

	typedef boost::multi_index_container<
		CachedHub, boost::multi_index::indexed_by<
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<tagAddress>, BOOST_MULTI_INDEX_MEMBER(CachedHub, Ip::Address, address)>,
			boost::multi_index::ordered_non_unique<
				boost::multi_index::tag<tagKey>, BOOST_MULTI_INDEX_MEMBER(CachedHub, uint32, key)>,
			boost::multi_index::ordered_non_unique<
				boost::multi_index::tag<tagSeen>, BOOST_MULTI_INDEX_MEMBER(CachedHub, std::time_t, seen), std::greater<std::time_t> > >
	> Cache;

public:

	HubCache(unsigned int);

	void SetCapacity(unsigned int);
	void Update(const G2::HubInfo&);
	void Touch(Ip::Endpoint, std::time_t ts = 0);
	void Remove(Ip::Address ip);
	void DiscardProxy(Ip::Address);

	G2::HubInfo Find(Ip::Address) const;
	G2::HubInfo GetRandomHub() const;
	G2::HubInfo GetHubWithoutKey() const;
	std::size_t Size() const;

	void Save(const std::string&);
	void Load(const std::string&);

	template <typename Predicate> 
	G2::HubInfo GetSearchableHub(Predicate pred) const
	{
		const std::time_t now = std::time(0);

		Cache::index_iterator<tagKey>::type i = cache_.get<tagKey>().upper_bound(0);

		for(; i != cache_.get<tagKey>().end(); ++i)
		{
			HubInfoPtr p = i->ptr;
			assert(p->QueryKey());
			if(now > p->SearchAfter() && pred(*p))
				return *p;
		}

		return G2::HubInfo();
	}

private:
	void Recache(HubInfoPtr);
	void Purge();

	Cache cache_;
	unsigned int maxHubs_;
};
	
#endif //HUBCACHE_HPP

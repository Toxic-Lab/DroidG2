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

#ifndef SHAREMANAGER_HPP
#define SHAREMANAGER_HPP

#include <map>
#include <set>
#include <stdexcept>
#include "types.hpp"
#include <list>
#include "sharedefs.hpp"
#include "sharecache.hpp"
#include "period.hpp"
#include "asynctask.hpp"
#include "sharesync.hpp"
#include "asynchash.hpp"
#include "idgen.hpp"
#include "managedinfo.hpp"
#include "queryhashtable.hpp"
#include <ctime>
#include "kwcache.hpp"

namespace Share {

class Cache;

class Manager
{
public:
	Manager();
	~Manager();
	void Start();
	//SFInfoPtr GetInfo(ID id);
//	void Attach(FileInfoPtr pInfo, EventsSubscriber* pSubscriber);
//	void Detach(FileInfoPtr pInfo, EventsSubscriber* pSubscriber);
	ManagedInfo Add(const FileInfo& info);
	ManagedInfo Add(const std::string& path);
	void Update(FileInfoPtr);
	void Recache(FileInfoPtr);
	void Remove(ID);
	void Remove(const std::string& path);
	void Shutdown();
	void OnTimer(std::time_t now);
	std::time_t QhtTime() const { return qhtTime_; }
	const QueryHashTable& Qht() const { return qht_; }

	template <typename Tag, typename Key>
	ManagedInfo Get(Key key)
	{
		return ManagedInfo(cache_.Get<Tag>(key), this);
	}

	ManagedInfo GetByHashes(const Hashes::SHA1&, const Hashes::TTR&, const Hashes::ED2K&, const Hashes::MD5&);
	ManagedInfo GetByUrn( const std::string& );
	void Rename(const std::string&, const std::string&);
	template <class T>
	void FindByDn(const std::string& dn, T out)
	{
		const std::size_t max = 30;
		std::vector<FileInfoPtr> results;
		kwcache_.Find(dn, std::back_inserter(results));
		for(std::size_t i = 0; i < results.size() && i < max; ++i)
			*out++ = ManagedInfo(results[i], this);
	}

	template <class T>
	void Dump(T out) 
	{
		for(Cache::iterator i = cache_.begin(); i != cache_.end(); ++i)
			*out++ = ManagedInfo(i->pInfo, this);
	}

private:
	//typedef std::set <EventsSubscriber*> SubscribersPtrs;
	//typedef std::map <FileInfoPtr, SubscribersPtrs> SubscribersContainer; 

	void Add(FileInfoPtr);
	void Remove(FileInfoPtr);
	bool Load();
	bool Save();
	//void Notify(FileInfoPtr p, int event);
	void Sync();
	void OnPathsSynced(const Sync::Result& result);
	void HashSomething();
	void OnHashed(const async_hash::Result&);
	void UpdateQHT(FileInfoPtr);

	template <typename T>
	bool ValidateHashUpdate(const T& h1, const T& h2)
	{
		if(h1 != h2)
		{
			if(h1.Empty()) return true;
			else throw NotPermited("Hash is already set");
		}
		return false;
	}

	//SubscribersContainer subscribers_;
	Cache cache_;
	KeywordCache<FileInfoPtr> kwcache_;
	Time::Period periodSync_;
	async_task::Work work_;
	Misc::IDGenerator idgen_;
	QueryHashTable qht_;
	QueryHashTable activeQht_;
	std::time_t qhtTime_;
	Time::Period periodSave_;
};

} //namespace Share
#endif //SHAREMANAGER_HPP


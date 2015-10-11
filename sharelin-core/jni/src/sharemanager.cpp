/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2010 Andrey Stroganov <savthe@gmail.com>

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

#include <vector>
#include <iomanip>
#include <fstream>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "sharemanager.hpp"
#include "settings.hpp"
#include "conv/conv.hpp"
#include "foreach.hpp"
#include "sharecache.hpp"
#include "asynctaskserver.hpp"
#include "filesystem.hpp"
#include "system.hpp"

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::iequals;
using boost::trim_copy;
using boost::istarts_with;

namespace Share {

Manager::Manager():
	periodSync_(Time::Minutes(3 * 60)),
	qhtTime_(0),
	periodSave_(Time::Minutes(10))
{
}

Manager::~Manager()
{
}

void Manager::Start()
{
	work_.Attach(System::GetTaskServer());
	System::LogBas() << "Starting share manager" << std::endl;
	Load();
	System::LogBas() << "We have " << cache_.Size() << " file(s) in share." << std::endl;
	if(cache_.Empty())
		System::LogBas() << "We should share something to support the G2 network." << std::endl; 
}

ManagedInfo Manager::GetByHashes(const Hashes::SHA1& sha1, const Hashes::TTR& ttr, const Hashes::ED2K& ed2k, const Hashes::MD5& md5)
{
	if( !sha1.Empty() )
	{
		try { return Get<tags::sha1>(sha1); } 
		catch(NotFound&) {}
	}

	if( !ttr.Empty() )
	{
		try { return Get<tags::ttr>(ttr); } 
		catch(NotFound&) {}
	}
	
	if( !ed2k.Empty() )
	{
		try { return Get<tags::ed2k>(ed2k); } 
		catch(NotFound&) {}
	}

	if( !md5.Empty() )
	{
		try { return Get<tags::md5>(md5); } 
		catch(NotFound&) {}
	}

	throw NotFound();
}

ManagedInfo Manager::GetByUrn( const std::string& urn )
{
	try
	{
		const std::size_t pos = urn.find_last_of( ':' );
		if( pos == std::string::npos ) throw NotFound();

		if( istarts_with( urn, "urn:sha1:" ) || istarts_with( urn, "sha1:" ) )
			return Get< tags::sha1 >( Hashes::SHA1( urn.substr( pos + 1 ) ) );

		if( istarts_with( urn, "urn:ttr:" ) || istarts_with( urn, "ttr:" ) )
			return Get< tags::ttr >( Hashes::TTR( urn.substr( pos + 1 ) ) );

		if( istarts_with( urn, "urn:ed2k:" ) || istarts_with( urn, "ed2k:" ) )
			return Get< tags::ed2k >( Hashes::ED2K( urn.substr( pos + 1 ) ) );

		if( istarts_with( urn, "urn:md5:" ) || istarts_with( urn, "md5:" ) )
			return Get< tags::md5 >( Hashes::MD5( urn.substr( pos + 1 ) ) );

		throw NotFound();
	}
	catch( std::exception& ) { throw NotFound(); }
	
	return ManagedInfo();
}

void Manager::Sync()
{
	System::LogBas() << "Scanning shared folders" << std::endl;
	Sync::Task t(boost::bind(&Manager::OnPathsSynced, this, _1));

	foreach(const std::string& path, System::GetSettings()->share.folders)
	{
		if(FileSystem::Exists(path))
			t.Scan(path);
		else
			System::LogBas() << "Shared path doesn't exist: " << path << std::endl;
	}

	std::vector<FileInfoPtr> ptrs;
	cache_.Dump(std::back_inserter(ptrs));
	foreach(FileInfoPtr p, ptrs)
		if(p->partial == 0) t.Validate(p->path);

	work_.Run(t);
}

void Manager::OnPathsSynced(const Sync::Result& result)
{
	System::LogBas() << "Shared folders scanned. Applying changes." << std::endl;

	if(!result.errors.empty())
	{
		System::LogBas() << "There were following errors during folders scan: " << std::endl;
		foreach(const Sync::Error& e, result.errors)
			System::LogBas() << e.path << " => " << e.message << std::endl;
	}

	uint removedCounter = 0;
	uint createdCounter = 0;

	foreach(const std::string& s, result.removed)
	{
		try 
		{
			Remove(s);
			++removedCounter;
		}
		catch (std::exception&) {}
	}

	foreach(const std::string& s, result.created)
	{
		try
		{
			if(FileSystem::IsRegularFile(s))
			{
				FileInfo info;
				info.path = s;
				Add(info);
				++createdCounter;
			}
		}
		catch(std::exception& e)
		{
			System::LogBas() << "There was a problem while adding " << s << " to the share => " << e.what() << std::endl;
		}
	}

	System::LogBas() << "New: " << createdCounter << std::endl;
	System::LogBas() << "Removed: " << removedCounter << std::endl;
	System::LogBas() << "Total: " << cache_.Size() << std::endl;

	Save();

	HashSomething();
}

void Manager::HashSomething()
{
	if( work_.Busy() ) return;

	const std::time_t now = System::Now();
	bool found = false;
	for(Cache::iterator i = cache_.begin(); i != cache_.end();)
	{
		FileInfoPtr p = i->pInfo;
		if(boost::starts_with(p->path, System::GetSettings()->general.incomplete))
		{
			++i;
			continue;
		}

		try
		{
			if(now - p->hashed > HASH_VALID_TIME)
			{
				found = true;
				System::LogBas() << "Hashing file: " << p->path << std::endl;
				FileRange range(0, FileSystem::FileSize(p->path));
				const uint rate = System::GetSettings()->share.hashRate * 1024 * 1024;
				work_.Run( async_hash::Task(p->path, Hashes::Mask::ANY, range, boost::bind(&Manager::OnHashed, this, _1), rate));
				break;
			}
			++i;
		}
		catch(std::exception& e)
		{
			System::LogBas() << "Removing shared item: " << p->path << " Reason: " << e.what() << std::endl;
			Remove(p);
			i = cache_.begin();
		}

	}

	if(!found)
	{
		qht_.Reset();
		for(Cache::iterator i = cache_.begin(); i != cache_.end(); ++i)
			UpdateQHT(i->pInfo);
	}
}

void Manager::Rename(const std::string& from, const std::string& to)
{
	FileInfoPtr p = cache_.Get<tags::path>(from);
	p->path = to;
	Remove(p);
	Add(p);
	UpdateQHT(p);
}

void Manager::OnHashed(const async_hash::Result& result)
{
	System::LogBas() << "File " << result.path << " hashed. Rate: " << std::setprecision(2) << double(result.rate) / (1024*1024) << " Mbps" << std::endl;
	try
	{
		FileInfoPtr p = cache_.Get<tags::path>(result.path);

		p->sha1 = result.sha1;
		p->ttr = result.ttr;
		p->ed2k = result.ed2k;
		p->md5 = result.md5;
		p->hashed = System::Now();
		p->size = result.range.Length();

		cache_.Update(p);
		UpdateQHT(p);

	}
	catch (NotFound&) 
	{
		std::cout << "NOT FOUND: " << result.path << std::endl;
	}
	HashSomething();
}

void Manager::UpdateQHT(FileInfoPtr p)
{
	qhtTime_ = System::Now();

	qht_.Add("urn:sha1:" + p->sha1.ToString()); 
	qht_.Add("urn:ttr:" + p->ttr.ToString()); 
	qht_.Add("urn:tree:tiger/:" + p->ttr.ToString()); 
	qht_.Add("urn:ed2k:" + p->ed2k.ToString()); 
	qht_.Add("urn:md5:" + p->md5.ToString()); 

	std::set<std::string> kws;
	G2::ExtractKeywords(p->name, std::inserter(kws, kws.begin()));
	foreach(const std::string& kw, kws) 
		qht_.Add(kw);
}

void Manager::OnTimer(std::time_t now)
{
	if(!work_.Busy() && periodSync_.Set())
		Sync();

	if( periodSave_.Set() )
		Save();
	/*
	if(periodWatchPointers_.Set())
	{
		for(PtrsContainer::iterator i = inuse_.begin(); i != inuse_.end();)
		{
			cache_.Update(*i);
			if(i->unique())
			{
				ptrs_.insert(*i);
				PtrsContainer::iterator next = i;
				++next;
				inuse_.erase(i);
				i = next;
			}
			else ++i;
		}
	}
	*/
}

void Manager::Shutdown()
{
	Save();
}
/*
void Manager::Attach(FileInfoPtr pInfo, EventsSubscriber* pSubscriber)
{
	subscribers_[pInfo].insert(pSubscriber);
}
*/

void Manager::Add(FileInfoPtr p)
{
	cache_.Add(p);
	kwcache_.Add(p, p->MakeName());
}

void Manager::Remove(FileInfoPtr p)
{
	kwcache_.Remove(p);
	cache_.Remove(p);
}

ManagedInfo Manager::Add(const FileInfo& info)
{
	FileInfoPtr p(new FileInfo(info));
	p->id = idgen_.Get();
	Add(p);
	return ManagedInfo(p, this);
}

ManagedInfo Manager::Add(const std::string& path)
{
	FileInfo inf;
	inf.path = path;
	return Add(inf);
}

void Manager::Remove(ID id)
{
	Remove(cache_.Get<tags::id>(id));
}

void Manager::Remove(const std::string& path)
{
	Remove(cache_.Get<tags::path>(path));
}

void Manager::Recache(FileInfoPtr p)
{
	cache_.Update(p);
}

/*FileInfoPtr Manager::FindByPath(const std::string& path)
{
	//return Find<tags::path>(path);
	PtrsContainer::iterator i;

	for(i = ptrs_.begin(); i != ptrs_.end(); ++i)
		if((*i)->path == path) break;

	if(i == ptrs_.end()) throw NotFound();

	return *i;
}
*/

/*
void Manager::Detach(FileInfoPtr pInfo, EventsSubscriber* pSubscriber)
{
	SubscribersContainer::iterator i = subscribers_.find(pInfo);
	if(i != subscribers_.end())
	{
		i->second.erase(pSubscriber);
		if(i->second.empty())
			subscribers_.erase(i);
	}
}

void Manager::Notify(FileInfoPtr p, int event)
{
	SubscribersContainer::iterator i = subscribers_.find(p);
	if(i != subscribers_.end())
	{
		std::vector<EventsSubscriber*> ptrs(i->second.begin(), i->second.end());
		std::for_each(ptrs.begin(), ptrs.end(), boost::bind(&EventsSubscriber::OnEvent, _1, event));
	}
}
*/

/*
FileInfoPtr Manager::GetInfo(ID id)
{
	SharedFiles::iterator i = files_.find(id);

	if(i != files_.end()) return i->second;
	else throw NotFound();	
}*/

bool Manager::Load()
{
	const std::string fname = System::Home() + "/share.dat";

	std::ifstream f(fname.c_str());
	if(!f) return false;

	std::string line;
	int lineNum = 0;
	while(f)
	{
		while(std::getline(f, line) && !boost::iequals(line, "[file]")) ++lineNum;
		
		FileInfoPtr p(new FileInfo);

		std::string strTiger;
		while(std::getline(f, line) && !boost::iequals(line, "[/file]"))
		{
			++lineNum;
			std::string::size_type sep = line.find(":");
			if(sep == std::string::npos) continue;
			std::string first = trim_copy(line.substr(0, sep));
			std::string second = trim_copy(line.substr(sep + 1));

			try
			{
				if(iequals(first, "path"))
					p->path = second;
				else if(iequals(first, "name"))
					p->name = second;
				else if(iequals(first, "sha1"))
					p->sha1.FromString(second);
				else if(iequals(first, "ttr"))
					p->ttr.FromString(second);
				else if(iequals(first, "md5"))
					p->md5.FromString(second);
				else if(iequals(first, "ed2k"))
					p->ed2k.FromString(second);
				else if(iequals(first, "hits"))
					p->hits = lexical_cast<uint>(second);
				else if(iequals(first, "size"))
					p->size = lexical_cast<int64>(second);
				else if(iequals(first, "requests"))
					p->requests = lexical_cast<uint>(second);
				else if(iequals(first, "hashed"))
					p->hashed = lexical_cast<std::time_t>(second);
				else if(iequals(first, "tigertree"))
					strTiger = second;
				else 
					System::LogBas() << "Unknown field: " << first << std::endl;
			}
			catch(std::exception& err)
			{
				System::LogBas() << "share.dat: " << lineNum << ": " << err.what() << std::endl;
			}
		}

		if(p->path.empty()) continue;

		if(p->size > 0 && !strTiger.empty())
		{
			std::vector<char> v;
			Conv::Base32::Decode(strTiger.begin(), strTiger.end(), std::back_inserter(v));
			p->tigerTree.FromBytes(v, p->size);
		}

		p->id = idgen_.Get();
		Add(p);
		UpdateQHT(p);
	}
	f.close();

	return true;
}

bool Manager::Save()
{
	const std::string fname = System::Home() + "/share.dat";

	std::vector<FileInfoPtr> ptrs;
	cache_.Dump(std::back_inserter(ptrs));

	std::ofstream f(fname.c_str());
	if(!f) return false;

	for(uint i = 0; i < ptrs.size(); ++i)
	{
		FileInfoPtr p = ptrs[i];
		f << "[file]" << std::endl;
		f << "path: " << p->path << std::endl;
		f << "name: " << p->name << std::endl;
		f << "sha1: " << p->sha1.ToString() << std::endl;
		f << "ttr: " << p->ttr.ToString() << std::endl;
		f << "md5: " << p->md5.ToString() << std::endl;
		f << "ed2k: " << p->ed2k.ToString() << std::endl;
		f << "size: " << p->size << std::endl;
		f << "hashed: " << p->hashed << std::endl;
		f << "hits: " << p->hits << std::endl;
		f << "requests: " << p->requests << std::endl;
		std::vector <char> v; 
		p->tigerTree.ToBytes(v);
		std::string s;
		Conv::Base32::Encode(v.begin(), v.end(), std::back_inserter(s));
		f << "tigertree: " << s << std::endl;
		f << "[/file]" << std::endl;
	}
	f.close();
	return true;
}

} //namespace Share

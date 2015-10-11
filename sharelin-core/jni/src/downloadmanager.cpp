/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009  Andrey Stroganov <savthe@gmail.com>

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

#include "downloadmanager.hpp"
#include "settings.hpp"
#include <set>
#include <assert.h>
#include <boost/lexical_cast.hpp>
#include "filesystem.hpp"
#include <algorithm>
#include "sharemanager.hpp"
#include "filemap.hpp"
#include "download.hpp"
#include "foreach.hpp"
#include <stdexcept>
#include "system.hpp"
#include "trafmanager.hpp"
#include "downloadrequest.hpp"

using boost::lexical_cast;
using FileSystem::ScanDirectory;
using FileSystem::IsRegularFile;

namespace {

inline int GetWeight(DownloadPtr p)
{
	enum {LOW = 0, AVERAGE = 500, HIGH = 1000};

	int w = 0;
	switch(p->Status())
	{
		case Download::HASH:
		case Download::MOVE:
		case Download::ERROR:
			w = HIGH;
			break;
		case Download::STOP:
		case Download::TRANSFER:
			w = AVERAGE + p->Priority();
			break;
		case Download::PAUSE:
			w = LOW;
			break;
	}

	return w;
}

bool CompareDownloads(DownloadPtr p1, DownloadPtr p2)
{
	return GetWeight(p1) > GetWeight(p2);
	/*
	const Download::StatusType s1 = p1->Status();
	const Download::StatusType s2 = p2->Status();
	
	enum Weigth {HIGH, MIDDLE, LOW};
	int w1
	if(s1 == Download::HASH || s1 == Download::MOVE || s1 == Download::ERROR) return true;
	if(s1 == Download::STOP || s1 == Download::PAUSE) return false;
	if(s2 == Download::HASH || s2 == Download::MOVE || s2 == Download::ERROR) return false;
	

	if(s1 == s1 == Download::ERROR || s1 == Download::MOVE || s1 == Download::HASH)
		return true;

	if(s1 == Download::TRANSFER && (s2 == Download::TRANSFER || s2 == Download::STOP) && p1->Priority() >= p2->Priority())
		return true;

	if((s1 == Download::TRANSFER || s1 == Download::STOP) && (s2 == Download::TRANSFER || s2 == Download::STOP))
		return p1->Priority() > p2->Priority();

	return false;
	*/
}

} //namespace 

DownloadManager::DownloadManager()
{
}

void DownloadManager::Detach(DownloadPtr p)
{
	for(uint i = 0; i < downloads_.size(); ++i)
		if(downloads_[i] == p)
		{
			downloads_.erase(downloads_.begin() + i);
			break;
		}
	Update();
}

void DownloadManager::OnTimer()
{
	DownloadsContainer::iterator i = downloads_.begin();
	while(i != downloads_.end() && (*i)->Status() == Download::TRANSFER && System::GetTraffIn()->Amount())
	{
		(*i)->OnTimer();
		++i;
	}
}

DownloadPtr DownloadManager::Create(const DownloadRequest& req)
{
	for(DownloadsContainer::iterator i = downloads_.begin(); i != downloads_.end(); ++i)
	{
		Share::ManagedInfo inf = (*i)->GetFileInfo();
		if(
			(!req.sha1.Empty() && req.sha1 == inf.Sha1()) ||
			(!req.ttr.Empty() && req.ttr == inf.Ttr()) ||
			(!req.md5.Empty() && req.md5 == inf.Md5()) ||
			(!req.ed2k.Empty() && req.ed2k == inf.Ed2k()) 
		  )
		{
			throw std::runtime_error("Alredy downloaded");
		}
	}

	if(req.sha1.Empty() && req.ttr.Empty() && req.md5.Empty() && req.ed2k.Empty())
		assert(!"Can't be so");

	std::string name;
	if(!req.sha1.Empty()) name = "sha1_" + req.sha1.ToString();
	else if(!req.ttr.Empty()) name = "ttr_" + req.ttr.ToString();
	else if(!req.md5.Empty()) name = "md5_" + req.md5.ToString();
	else if(!req.ed2k.Empty()) name = "ed2k_" + req.ed2k.ToString();
	else throw std::runtime_error("No hash");

	Share::FileInfo info;
	info.partial = idgen_.Get();
	info.name = req.name.empty() ? name : req.name;
	info.path = System::GetSettings()->general.incomplete + "/" + name;
	info.size = req.size;
	info.sha1 = req.sha1;
	info.ttr = req.ttr;
	info.md5 = req.md5;
	info.ed2k = req.ed2k;

	std::fstream file;
	file.open(info.path.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
	if(!file.good())
		throw std::runtime_error("Can't create file " + info.path);
	file.close();

	try { System::GetShareMgr()->Add(info); }
	catch (Share::AlreadyShared& )
	{
		throw std::runtime_error("Can't create file info" + info.path);
	}

	DownloadPtr pDownload(new Download(info.partial));
	pDownload->Open(info.path);
	downloads_.push_back(pDownload);
	pDownload->SaveParams();
	Update();

	return pDownload;
}

DownloadPtr DownloadManager::Get(ID id)
{
	DownloadPtr p;
	foreach(DownloadPtr p, downloads_)
		if(p->GetFileInfo().Partial() == id) return p;

	throw std::runtime_error("Bad ID");
}

void DownloadManager::Start()
{
	System::LogBas() << "Loading downloads" << std::endl;
	typedef std::vector<std::string> FileNames;
	FileNames files;
	ScanDirectory(System::GetSettings()->general.incomplete, std::back_inserter(files), FileSystem::REGULAR_FILE, FileSystem::NOT_FOLLOW);

	for(FileNames::iterator i = files.begin(); i != files.end(); ++i) 
	{
		ID id = idgen_.Get();
		DownloadPtr pDownload(new Download(id));
		try { pDownload->Open(*i); }
		catch (std::runtime_error& e)
		{
			System::LogBas() << "Can't create download for file: " << *i << " Error: " << e.what() << std::endl;
			continue;
		}
		pDownload->GetFileInfo().Partial(id);
		downloads_.push_back(pDownload);
	}	

	Update();

	System::LogBas() << lexical_cast<std::string>(downloads_.size()) << " download(s) created." << std::endl;
}

void DownloadManager::Update()
{
	std::stable_sort(downloads_.begin(), downloads_.end(), CompareDownloads);
	uint active = System::GetSettings()->net.maxFiles;

	std::size_t i = 0;
	for(i = 0; i < downloads_.size() && active; ++i)
	{
		DownloadPtr p = downloads_[i];
		if(p->Status() == Download::STOP)
			p->Resume();
		if(p->Status() == Download::TRANSFER)
			--active;
	}

	for(; i < downloads_.size(); ++i)
	{
		DownloadPtr p = downloads_[i];
		if(p->Status() == Download::TRANSFER)
			p->Stop();
	}
}

void DownloadManager::Shutdown()
{
	foreach(DownloadPtr p, downloads_) p->Stop();
	downloads_.clear();
}


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

#ifndef DOWNLOAD_HPP
#define DOWNLOAD_HPP

#include <set>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "sharedefs.hpp"
#include "types.hpp"
#include "asynctask.hpp"
#include "period.hpp"
#include "sharedfile.hpp"
#include "filemapptr.hpp"
#include "filemap.hpp"
#include "tigertree.hpp"
#include "asynchash.hpp"
#include "asynccopy.hpp"
#include "ip.hpp"
#include "guid.hpp"
#include "netfwd.hpp"
#include "managedinfo.hpp"
#include "searchdefs.hpp"
#include "sourcehost.hpp"

namespace Search { class Searcher; }

class Gatekeeper;

class Download: public boost::enable_shared_from_this<Download>
{
public:
	enum StatusType {TRANSFER, HASH, MOVE, PAUSE, STOP, ERROR};
	enum PriorityType {LOW = 0, BELOW, NORMAL, ABOVE, HIGH};

	Download(ID);
	StatusType Status() const { return status_; }
	uint Priority() const { return priority_; }
	void Priority(unsigned); 
	ID GetId() const { return id_; }
	Share::ManagedInfo GetFileInfo() const { return fileInfo_; }
	FileMapPtr GetFileMap() { return pFileMap_; }
	uint AvrRate() const;
	uint CountSources() const { return sources_.size(); }
	uint CountDownloaders() const { return downloaders_.size(); }
	std::string DefaultURN() const;

	bool HandleGIV(Gatekeeper*, GUID);
	void Open(const std::string& path);
	void Resume();
	void Stop();
	void Pause();
	void OnTimer();
	void Detach(DownloaderPtr);
	void OnIdle(DownloaderPtr);
	void Cancel();
	void AddSource(const SourceHost& host);
	void OnFileSize(file_offset_t size);
	void SaveParams();
	void FileUpdated();
	void OnTiger(const Hashes::TigerTree& t);

	template <typename T> void Dump(T out) 
	{ 
		std::copy(downloaders_.begin(), downloaders_.end(), out); 
	}

	template <typename T> void CopySources(T out)
	{
		for(SourcesContainer::iterator i = sources_.begin(); i != sources_.end(); ++i)
			*out++ = *(i->second);
	}

private:
	typedef std::set<DownloaderPtr> DownloadersContainer;
	typedef std::map<Ip::Endpoint, SourceHostEx::Ptr> SourcesContainer;

	void LoadParams();
	void FileComplete();
	void CloseDownloaders();
	void CancelTasks();
	void ConnectSource();
	void Hashed(const async_hash::Result&);
	void Copied(const async_copy::Result&);
	void DetachMe();
	bool AllowDownloader() const;
	void Push(SourceHostEx::Ptr);
	std::string InfPath();

	static uint momentDlrs_;
	static std::time_t moment_;

	ID id_;
	Share::ManagedInfo fileInfo_;
	FileMapPtr pFileMap_;
	StatusType status_;
	uint priority_;
	SharedFile file_;
	Time::Period periodSources_;
	Time::Period periodSave_;
	boost::shared_ptr<Search::Searcher> pSearcher_;
	SourcesContainer sources_;
	DownloadersContainer downloaders_;
	async_task::Work work_;
};


#endif //DOWNLOAD_HPP


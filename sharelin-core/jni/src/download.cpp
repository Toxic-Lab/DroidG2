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

#include "download.hpp"
#include <boost/algorithm/string.hpp>
#include "filesystem.hpp"
#include "hashes.hpp"
#include "sharemanager.hpp"
#include <sstream>
#include <fstream>
#include "settings.hpp"
#include "conv/conv.hpp"
#include <vector>
#include "asynctaskserver.hpp"
#include "downloader_impl.hpp"
#include "foreach.hpp"
#include "filemanager.hpp"
#include "searcher.hpp"
#include "searchmanager.hpp"
#include "filemap.hpp"
#include "asynchash.hpp"
#include "downloadmanager.hpp"
#include <stdexcept>
#include "system.hpp"
#include "trafmanager.hpp"
#include "g2net.hpp"
#include "gatekeeper.hpp"
#include "history.hpp"

using FileSystem::FileName;
using FileSystem::FileSize;
using FileSystem::MakeAvailableName;
using FileSystem::Unlink;
using boost::starts_with;
using boost::iequals;

uint Download::momentDlrs_ = 0;
std::time_t Download::moment_ = 0;

namespace {

const uint DOWNLOADERS_PER_SECOND = 20;
const std::time_t SOURCE_REST_TIME = 1 * 60;

} //namespace


Download::Download(ID id):
	id_(id),
	pFileMap_(new FileMap),
	status_(STOP),
	priority_(NORMAL),
	periodSources_(Time::Seconds(5)),
	periodSave_(Time::Minutes(5)),
	work_(System::GetTaskServer())
{
}

void Download::Open(const std::string& path)
{
	const std::string fileName = FileName(path);

	Hashes::SHA1 sha1;
	Hashes::TTR ttr;
	Hashes::MD5 md5;
	Hashes::ED2K ed2k;

	try
	{
		if(starts_with(fileName, "sha1_"))
			sha1.FromString(fileName.substr(5));
		else if(starts_with(fileName, "ttr_"))
			ttr.FromString(fileName.substr(4));
		else if(starts_with(fileName, "md5_"))
			md5.FromString(fileName.substr(4));
		else if(starts_with(fileName, "ed2k_"))
			ed2k.FromString(fileName.substr(5));
		else throw std::runtime_error("temp file name");
	}
	catch (HashConversionError&)
	{
		throw std::runtime_error("temp file name conversion");
	}

	assert( !(sha1.Empty() && ttr.Empty() && md5.Empty() && ed2k.Empty()) );

	const file_offset_t fileSize = FileSize(path);

	try { fileInfo_ = System::GetShareMgr()->Get<Share::tags::path>(path); }
	catch (Share::NotFound& )
	{
		System::LogBas() << "No shared info for " << path << " creating" << std::endl;
		Share::FileInfo info;
		info.name = FileName(path);
		info.path = path;
		info.size = fileSize;
		info.sha1 = sha1;
		info.ttr = ttr;
		info.md5 = md5;
		info.ed2k = ed2k;
		try { fileInfo_ = System::GetShareMgr()->Add(info); }
		catch (Share::AlreadyShared& )
		{
			assert(!"Should not be here");
		}
	}
	
	OnFileSize(fileInfo_.Size());

	LoadParams();
}

void Download::Pause()
{
	Stop();
	status_ = PAUSE;
}

void Download::Priority(unsigned n)
{
	if(n > HIGH) throw std::logic_error("Priority is too high");
	priority_ = n;
}

void Download::Cancel()
{
	Stop();
	Unlink(InfPath());
	Unlink(fileInfo_.Path());
	System::GetShareMgr()->Remove(fileInfo_.Id());
	DetachMe();
}

void Download::Push(SourceHostEx::Ptr pHost)
{
	if(System::IsFirewalled()) return;
	if(pHost->guid.Empty()) return;
	if(pHost->hubs.empty()) return;

	const Ip::Endpoint hub = pHost->hubs[rand() % pHost->hubs.size()];

	System::LogAdv() << "Pushing via " << hub << std::endl;
	Ip::Endpoint self = System::SelfEndpoint();
	G2::Packet push;
	G2::Writer w(push);

	w.Begin(G2::PUSH);

	w.Begin(G2::TO);
	w.Write(pHost->guid.Get(), 16);
	w.Close(G2::TO);

	w.Pod(self.address.value);
	w.Pod(self.port);

	w.Close(G2::PUSH);

	/*push.payload.write_pod(self.address.value);
	push.payload.write_pod(self.port);
	G2::PacketIterator i = push.packets.add(G2::PacketType::TO);
	i->payload.write(guid.Get(), guid.Get() + 16);
	*/
	System::GetNetwork()->Send(push, hub, Ip::Protocol::UDP);
}

bool Download::HandleGIV(Gatekeeper* pGatekeeper, GUID guid)
{
	for(SourcesContainer::iterator i = sources_.begin(); i != sources_.end(); ++i)
	{
		SourceHostEx::Ptr p = i->second;
		if(p->guid == guid)
		{
			if(AllowDownloader() && p->connections <= p->maxConnections)
			{
				DownloaderPtr pDownloader(new DownloaderImpl(*this, pGatekeeper->GetSocketPtr(), p));
				downloaders_.insert(pDownloader);
				OnIdle(pDownloader);
				++momentDlrs_;
			}
			return true;
		}
	}
	/*
	if(sources_.find(guid) != sources_.end())
	{
		SourceHostEx::Ptr pHost = sources_[guid];
		if(AllowDownloader() && pHost->connections <= pHost->maxConnections)
		{
			DownloaderPtr p(new Downloader(*this, pGatekeeper->GetSocketPtr(), pHost));
			downloaders_.insert(p);
			OnIdle(p);
		}
		return true;
	}
	*/
	return false;
}

void Download::OnFileSize(file_offset_t size)
{
	if(pFileMap_->Empty())
	{
		try 
		{ 
			char empty = 0;
			AsyncFile file;
			file.Open(fileInfo_.Path(), fio::rw); 
			file.Write(&empty, 1, size - 1);
		}
		catch(AsyncFile::Error&)
		{
			System::LogBas() << "Can't resize file: " << fileInfo_.Path() << " to " << size << " bytes" << std::endl;
			status_ = ERROR;
			return;
		}
		pFileMap_->Init(size);
	}
}

bool Download::AllowDownloader() const
{
	return 
		status_ == TRANSFER && 
		momentDlrs_ < DOWNLOADERS_PER_SECOND &&
		DownloaderImpl::Count() < System::GetSettings()->net.maxDownloads&&
		!System::GetTraffIn()->ThresholdReached() && 
		(pFileMap_->Status() & (Integrity::EMPTY | Integrity::INVALID));
}

void Download::ConnectSource()
{
	SourceHostEx::Ptr pSource;
	for(SourcesContainer::iterator i = sources_.begin(); i != sources_.end(); ++i)
	{
		SourceHostEx::Ptr p = i->second;
		if(System::Now() < p->connectAfter) continue;
		if(p->connections >= p->maxConnections) continue;

		pSource = p;
		break;
	}

	if(!pSource) return;

	System::LogAdv() << "Connecting to " << pSource->endpoint << " to download " << fileInfo_.Name() << std::endl;

	++momentDlrs_;
	DownloaderPtr pDownloader(new DownloaderImpl(*this, pSource));
	downloaders_.insert(pDownloader);
	pDownloader->Connect();

}


void Download::FileUpdated()
{
	if(work_.Busy()) return;

	const Integrity::Type s = pFileMap_->Status();

	if(s == Integrity::VALID || s == Integrity::INVALID)
	{
		assert(!work_.Busy());
		CloseDownloaders();
		FileComplete();
	}
	else if(s == Integrity::FULL)
	{
		CloseDownloaders();
		status_ = HASH;
		file_.Sync();
		work_.Run(async_hash::Task(fileInfo_.Path(), Hashes::Mask::ANY, FileRange(0, fileInfo_.Size()),
					boost::bind(&Download::Hashed, this, _1)));
	}
	else if(s & Integrity::FULL)
	{
		if(!fileInfo_.TigerTreeHash().Empty())
		{
			FileRange r = pFileMap_->GetForValidation();
			if( !r.Empty() )
			{
				file_.Sync();
				work_.Run(async_hash::Task(fileInfo_.Path(), Hashes::Mask::TTR, r, boost::bind(&Download::Hashed, this, _1)));
			}
		}
	}
}

void Download::DetachMe()
{
	work_.Abort();
	CloseDownloaders();
	System::GetDownloadMgr()->Detach(shared_from_this());
}

void Download::FileComplete()
{
	std::string name = fileInfo_.Name();
	if(name.empty()) name = DefaultURN();
	if(pFileMap_->Status() == Integrity::INVALID)
	{
		System::LogBas() << "File " << name << " has been downloaded, but may be corrupted. Moving." << std::endl;
		name = "CORRUPTED_" + name;
	}
	else
	{
		assert(pFileMap_->Status() == Integrity::VALID);
		System::LogBas() << "File " << name << " has been successfully downloaded. Moving." << std::endl;
	}

	status_ = MOVE;
	file_.Sync();
	std::string newName = MakeAvailableName(System::GetSettings()->general.complete + "/" + name);
	System::LogBas() << "Copying file " << fileInfo_.Path() << " to " << newName << std::endl;
	work_.Run(async_copy::Task(fileInfo_.Path(), newName, boost::bind(&Download::Copied, this, _1)));
}

void Download::Copied(const async_copy::Result& result)
{
	if(result.status == async_copy::GOOD)
	{
		assert(FileSystem::Exists(result.to));
		Stop();
		try 
		{ 
			Unlink(InfPath());

			System::GetFileMgr()->Transfer(result.from, result.to); 
			System::GetShareMgr()->Rename(result.from, result.to);

			History::Download hist(result.to, fileInfo_.Size());
			System::GetHistory()->Add(hist);

			System::LogBas() << "File " << result.to << " is ready to use" << std::endl;

			Unlink(result.from);
		}
		catch ( std::exception& e )
		{
			System::LogBas() << "File moving error. Transfer problem: " << e.what() << std::endl;
			DetachMe();
		}
		//fileInfo_.Path(result.to);
		DetachMe();
	}
	else
	{
		System::LogBas() << "File copy error. From: " << result.from << " to: " << result.to << " error: " << result.message << std::endl;
		status_ = ERROR;
	}
}

void Download::Hashed(const async_hash::Result& result)
{
	assert(!work_.Busy());
	FileRange r = result.range;

	if(result.status == async_hash::FILE_ERROR)
	{
		System::LogBas() << "ERROR during part hash " << r.First() << "-" << r.Last() << std::endl;
		return;
	}

	assert(result.status == async_hash::GOOD);

	bool match = false;
	if(r.Length() == fileInfo_.Size())
	{
		assert(r.Offset() == 0);

		match |= !result.sha1.Empty() && result.sha1 == fileInfo_.Sha1(); 
		match |= !result.ttr.Empty() && result.ttr == fileInfo_.Ttr(); 
		match |= !result.md5.Empty() && result.md5 == fileInfo_.Md5(); 
		match |= !result.ed2k.Empty() && result.ed2k == fileInfo_.Ed2k(); 

	}
	else
	{
		assert(!result.ttr.Empty());
		match = result.ttr == fileInfo_.TigerTreeHash().Get(r);
	}

	if(match)
	{
	//	verb.bas << "Setting range VALID: " << r.First() << "-" << r.Last() << verb.flush;
		pFileMap_->SetValid(r);
	}
	else
	{
		System::LogAdv() << "Invalidating " << fileInfo_.Name() << " Range: " << r.First() << "-" << r.Last() << std::endl;
	//	verb.bas << result.ttr.ToString() << verb.flush;
	//	verb.bas << fileInfo_.tigerTree.Get(r).ToString() << verb.flush;
		pFileMap_->SetInvalid(r);
	}

	FileUpdated();
}

void Download::OnTiger(const Hashes::TigerTree& t)
{
	try
	{
		fileInfo_.TigerTreeHash(t);
		System::LogAdv() << "Got valid tiger tree. Minimal range: " << t.MinimalRange() << std::endl;
		pFileMap_->SetVerificationSize(t.MinimalRange());
	}
	catch (Share::NotPermited& err)
	{
		System::LogAdv() << "Failed to import tiger tree: " << err.what() << std::endl;
	}
}

void Download::OnTimer()
{
	//std::cout << Downloader::Count() << std::endl;
	if(status_ == TRANSFER)
	{
		std::vector<DownloaderPtr> ptrs;
		ptrs.reserve(downloaders_.size());
		std::copy(downloaders_.begin(), downloaders_.end(), std::back_inserter(ptrs));
		foreach(DownloaderPtr p, ptrs) p->OnTimer();

		assert(pSearcher_);
		std::vector<Search::Result> results;
		pSearcher_->DumpResults(std::back_inserter(results));
		pSearcher_->ClearResults();
		foreach(const Search::Result& r, results)
			AddSource(r.host);

		if(moment_ != System::Now())
		{
			moment_ = System::Now();
			momentDlrs_ = 0;
		}

		if(AllowDownloader()) ConnectSource();

		if(periodSave_.Set()) SaveParams();
	}
}

void Download::Detach(DownloaderPtr p)
{
	if(p->CompletedTasks() == 0)
	{
		SourceHostEx::Ptr pHost = p->HostInfo();
		pHost->connectAfter = System::Now() + SOURCE_REST_TIME;

		if(!p->Pushed()) Push(pHost);
	}

	downloaders_.erase(p);
}


void Download::OnIdle(DownloaderPtr p)
{
	if(status_ == TRANSFER && p->Task() == Downloader::FETCH_FILE)
		FileUpdated();

	if(status_ == TRANSFER)
	{
		if(p->HasTiger() && fileInfo_.TigerTreeHash().Empty())
			p->FetchTiger();
		else if(pFileMap_->Status() & (Integrity::EMPTY | Integrity::INVALID) )
		{
			std::string uri = p->HostInfo()->uri;
			if(uri.empty()) uri = "/uri-res/N2R?urn:" + DefaultURN();
			p->FetchFile(uri, file_, pFileMap_);
		}
		else 
		{
			p->Close();
			downloaders_.erase(p);
		}
	}
	else assert(downloaders_.empty());
}

std::string Download::DefaultURN() const
{
	if( !fileInfo_.Sha1().Empty() )
		return "sha1:" + fileInfo_.Sha1().ToString();
	if( !fileInfo_.Ed2k().Empty())
		return "ed2k:" + fileInfo_.Ed2k().ToString();
	if(!fileInfo_.Ttr().Empty())
		return "ttr:" + fileInfo_.Ttr().ToString();
	if(!fileInfo_.Md5().Empty())
		return "md5:" + fileInfo_.Md5().ToString();
	return std::string();
}

std::string Download::InfPath()
{
	return System::GetSettings()->general.incomplete + "/inf/" + FileName(fileInfo_.Path()) + ".inf";
}

void Download::LoadParams()
{
	std::ifstream f(InfPath().c_str());

	std::string line;
	while(std::getline(f, line))
	{
		std::stringstream ss(line);
		std::string first;
		if( !(ss >> first) ) continue;

		if(boost::iequals(first, "priority"))
			ss >> priority_;
		else if(boost::iequals(first, "status"))
		{
			std::string second;
			ss >> second;
			if(boost::equals(second, "paused")) status_ = PAUSE;
			else status_ = STOP;
		}
		else if(boost::iequals(first, "map"))
		{
			FileMap fmap;
			try { fmap.FromStream(ss); }
			catch (std::runtime_error&)
			{
				System::LogBas() << "Filemap load error from " << InfPath() << std::endl;
				continue;
			}

			if(!pFileMap_)
			{
				System::LogBas() << "Loaded file map. File is not ready. Fixing." << std::endl;
				OnFileSize(fmap.FileSize());
			}

			if(fmap.FileSize() != pFileMap_->FileSize())
			{
				System::LogBas() << "Size of loaded file map differs from file size. File will be redownloaded." << std::endl;
				continue;
			}

			*pFileMap_ = fmap;

			if(!fileInfo_.TigerTreeHash().Empty())
				pFileMap_->SetVerificationSize(fileInfo_.TigerTreeHash().MinimalRange());
		}
		else if(iequals(first, "source"))
		{
			std::string addr;
			ss >> addr;
			SourceHost src;
			if(src.endpoint.FromString(addr))
				AddSource(src);
		}
	}
}

void Download::SaveParams()
{
	std::ofstream f(InfPath().c_str());

	if(!f.good())
	{
		System::LogBas() << "Can't open file for writing: " << InfPath() << std::endl;
		return;
	}

	f << "priority " << priority_ << std::endl;
	
	f << "status ";
	switch (status_)
	{
		case PAUSE: f << "paused"; break;
		default: f << "transfer";
	}
	f << std::endl;

	if(!pFileMap_->Empty())
	{
		f << "map ";
		pFileMap_->ToStream(f);
		f << std::endl;
		/*
		std::cout << "TEST FILEMAP SERIALIZATION" << std::endl;
		std::stringstream s;
		pFileMap_->ToStream(s);
		FileMap fmap;
		fmap.FromStream(s);
		std::stringstream ss;
		fmap.ToStream(ss);
		assert(s.str() == ss.str());
		*/
	}

	for(SourcesContainer::iterator i = sources_.begin(); i != sources_.end(); ++i)
	{
		f << "source " << i->first << std::endl;
	}

	f.close();
}

uint Download::AvrRate() const
{
	uint rate = 0;
	foreach(DownloaderPtr p, downloaders_)
		rate += p->AvrRate();

	return rate;
}

void Download::Resume()
{
	assert(status_ == STOP || status_ == PAUSE);
	try
	{ 
		file_ = System::GetFileMgr()->Open(fileInfo_.Path(), fio::rw); 

		Search::Criteria crt;
		crt.sha1 = fileInfo_.Sha1();
		crt.ttr = fileInfo_.Ttr();
		crt.md5 = fileInfo_.Md5();
		crt.ed2k = fileInfo_.Ed2k();

		pSearcher_ = System::GetSearchMgr()->Create(crt, Search::Manager::AUTO);
	}
	catch (std::exception& e)
	{
		System::LogBas() << "Download resume error: " << e.what() << std::endl;
		status_ = ERROR;
		return;
	}
	status_ = TRANSFER;
}

void Download::Stop()
{
	System::GetSearchMgr()->Detach(pSearcher_);
	CloseDownloaders();
	CancelTasks();
	SaveParams();
	status_ = STOP;
}

void Download::CloseDownloaders()
{
	foreach(DownloaderPtr p, downloaders_)
		p->Close();
	downloaders_.clear();
}

void Download::CancelTasks()
{
	work_.Abort();
}

void Download::AddSource(const SourceHost& source)
{
	SourcesContainer::iterator i = sources_.find(source.endpoint);
	if(i == sources_.end())
		sources_[source.endpoint] = SourceHostEx::Ptr(new SourceHostEx(source));
	else
	{
		i->second->hubs = source.hubs;
		i->second->guid = source.guid;
	}
}


	

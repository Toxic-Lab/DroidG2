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

#include <fstream>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "webuicontrol.hpp"
#include "ui/command.hpp"
#include "ui/template.hpp"
#include "identity.hpp"
#include "g2node.hpp"
#include "media.hpp"
#include "searchmanager.hpp"
#include "searcher.hpp"
#include "settings.hpp"
#include "system.hpp"
#include "uicommons.hpp"
#include "g2net.hpp"
#include "downloadmanager.hpp"
#include "download.hpp"
#include "ui/sortprefs.hpp"
#include "filemap.hpp"
#include "foreach.hpp"
#include "downloader.hpp"
#include "url.hpp"
#include "sharedefs.hpp"
#include "searchdefs.hpp"
#include "downloadrequest.hpp"
#include "uploader.hpp"
#include "uploadmanager.hpp"
#include "history.hpp"
#include "filesystem.hpp"
#include "sharemanager.hpp"
#include "ui/prefs.hpp"
#include "webuiserver.hpp"
#include "uicomparers.hpp"
#include "trafmanager.hpp"
#include "g2handshake.hpp"
#include "updateswatch.hpp"

#include <android/log.h>

using boost::lexical_cast;
using boost::starts_with;
using boost::istarts_with;
using FileSystem::FileName;
using FileSystem::IsRegularFile;
using FileSystem::ScanDirectory;
using FileSystem::FileSize;
using FileSystem::Unlink;
using namespace Ui;

typedef boost::shared_ptr<Search::Searcher> SearcherPtr;

namespace {

std::string Tpl(const std::string& fname) 
{
	return System::GetWebUi()->GetStyleRoot() + "/" + fname + ".tpl";
}

std::string NameFromCriteria(const Search::Criteria& criteria)
{
	std::string dn = criteria.dn;
	if(!criteria.sha1.Empty())
		dn = "SHA1 hash: " + criteria.sha1.ToString();
	else if(!criteria.ttr.Empty())
		dn = "TTR hash: " + criteria.ttr.ToString();
	else if(!criteria.ed2k.Empty())
		dn = "ED2K hash: " + criteria.ed2k.ToString();
	else if(!criteria.md5.Empty())
		dn = "MD5 hash: " + criteria.md5.ToString();
	return dn;
}
	
struct ShareWriter
{
	ShareWriter(Ui::Template& tpl): tpl_(tpl) {}

	void operator()(Share::ManagedInfo inf)
	{
		tpl_.Put("name", inf.Name());
		tpl_.Put("path", inf.Path());
		tpl_.Put("size", Ui::HumanReadable(inf.Size()));
		tpl_.Put("type", Ui::FileType(inf.Name()));
		tpl_.Put("hits", inf.Hits());
		tpl_.Put("requests", inf.Requests());
	}
	Ui::Template& tpl_;
};

struct SearchResultWriter
{
	SearchResultWriter(Ui::Template& tpl, ID sid): tpl_(tpl), sid_(sid) 
	{
		tplGroup_.Load(Tpl("resultsgroup"));
	}

	void operator()(const Ui::SearchResultGroup& g)
	{
 		tplGroup_.ClearValues();
		size_t showNames = 2;
		std::string fileType;
 		for(Ui::SearchResultGroup::RankedNames::const_iterator iName = g.names.begin(); iName != g.names.end() && showNames; ++iName, --showNames)
 		{
 			tplGroup_.Put("name", Ui::AdaptString(iName->first));
 			tplGroup_.Put("rid", iName->second);
 			tplGroup_.Put("sid", sid_);
 			if(fileType.empty())
				fileType = Ui::FileType(iName->first);
 
 		}
 
 		tpl_.Put("name", tplGroup_);
 		tpl_.Put("size", Ui::HumanReadable(g.fileSize));
 		tpl_.Put("type", fileType);
		tpl_.Put("sources", g.sources);
	}

	Ui::Template& tpl_;
	Ui::Template tplGroup_;
	ID sid_;
};

template <typename Iter, typename Output> void PagedOutput(uint page, Iter begin, Iter end, Output out)
{
	const uint perPage = System::GetSettings()->webui.pageLines;
	const uint lower = page * perPage;
	const uint upper = (page + 1) * perPage - 1;
	uint count = 0;
	for(Iter i = begin; i != end; ++i, ++count)
		if(count >= lower && count < upper) out(*i);
}

template <typename T> void MakePageIterators(uint page, T& begin, T& end)
{
}

uint CountPages(uint elements, uint perPage)
{
	return elements % perPage ? elements / perPage + 1 : elements / perPage;
}

} //namespace

WebuiControl::WebuiControl(const Ui::Command& command, std::string& result):
	command_(command)
{
	try 
	{ 
		if(command_.Has(Ui::Tags::sort) && command_.Has(Ui::Tags::sortby))
		{
			const uint64 id = StaticName::FromString(command_.Get<std::string>(Ui::Tags::sort));
			const uint64 sortby = StaticName::FromString(command_.Get<std::string>(Ui::Tags::sortby));
			GetPrefs().sorting.Update(id, sortby);
		}

		Do(); 
	}
	catch(std::exception& e)
	{
		result = "Error occured: " + std::string(e.what());
		return;
	}

	page_.Format(result);
}

Ui::Prefs& WebuiControl::GetPrefs()
{
	return System::GetWebUi()->GetPrefs();
}

void WebuiControl::Page(Ui::Tags::Type name)
{
	if(name == Tags::status) page_.Load(Tpl("status"));
	else page_.Load(Tpl("page"));

	page_.Put("pgname", StaticName::ToString(name));
	if(command_.Has(Ui::Tags::pgreload))
		GetPrefs().reloadRate[name] = command_.Get<uint>(Ui::Tags::pgreload);
	page_.Put("reload", GetPrefs().reloadRate[name]);
}

void WebuiControl::CntCreateSearch()
{
	Search::Criteria criteria;

	const std::string dn = command_.Get<std::string>(Ui::Tags::dn);
	if(istarts_with(dn, "urn:sha1:"))
		criteria.sha1.FromString(dn.substr(9));
	else if(istarts_with(dn, "urn:ttr:"))
		criteria.ttr.FromString(dn.substr(8));
	else if(istarts_with(dn, "urn:ed2k:"))
		criteria.ed2k.FromString(dn.substr(9));
	else if(istarts_with(dn, "urn:md5:"))
		criteria.md5.FromString(dn.substr(8));
	else criteria.dn = dn;

	if(command_.Has(Ui::Tags::min))
		criteria.min = Ui::GetSizeValue(command_.Get<std::string>(Ui::Tags::min));
	if(command_.Has(Ui::Tags::max))
		criteria.max = Ui::GetSizeValue(command_.Get<std::string>(Ui::Tags::max));
	if(command_.Has(Ui::Tags::media))
	{
		if(command_.Get<std::string>(Ui::Tags::media) != "any")
			criteria.media = StaticName::FromString(command_.Get<std::string>(Ui::Tags::media));
	}
	if(command_.Has(Ui::Tags::ext))
	{
		std::stringstream s(command_.Get<std::string>(Ui::Tags::ext));
		std::string ext;
		while(s >> ext)
			criteria.extensions.insert(boost::to_lower_copy(ext));
	}
	System::GetSearchMgr()->Create(criteria, Search::Manager::USER);
}

void WebuiControl::CntSearchResults()
{
	SearcherPtr pSearcher;
	ID id = command_.Get<ID>(Ui::Tags::show);
	pSearcher = System::GetSearchMgr()->Get(id);

	typedef std::vector<Search::Result> ResultsContainer;
	ResultsContainer results;
	pSearcher->DumpResults(std::back_inserter(results));

	Search::Criteria criteria = pSearcher->GetCriteria();
	//Put("endless", criteria.endless);
	Put("dn", NameFromCriteria(criteria));
	Put("id", id);

	if(criteria.min)
		Put("min", Ui::HumanReadable(criteria.min));
	if(criteria.max)
		Put("max", Ui::HumanReadable(criteria.max));
	Put("extensions", Ui::FormatExtensions(criteria.extensions.begin(), 
				criteria.extensions.end()));

	Ui::SRGroupContainer groups;
	Ui::CreateOrderedSRGroups(results.begin(), results.end(), groups, GetPrefs().sorting.Option(Ui::Tags::sres));

	if(results.empty()) return;

	const uint perPage = System::GetSettings()->webui.pageLines;
	const uint pagen = command_.Has(Ui::Tags::page) ? command_.Get<uint>(Ui::Tags::page) - 1 : 0;
	const uint totalPages = CountPages(groups.size(), perPage);

	for(uint i = 1; i <= totalPages; ++i) Put("pagen", i);
	Put("page_cur", pagen + 1);
	Put("page_total", totalPages);

	PagedOutput(pagen, groups.begin(), groups.end(), SearchResultWriter(page_, id));
}

void WebuiControl::Init()
{
	if(System::GetSettings()->webui.pageLines == 0)
	{
		System::LogBas() << "Webui lines per page option is zero. Check configuration" << std::endl;
		System::GetSettings()->webui.pageLines = 100;
	}
}

void WebuiControl::Do()
{
	Init();
	switch(command_.Cmd())
	{
		case Ui::Tags::kill:
		{
			System::Shutdown();
			return;
		}

		case Ui::Tags::home: 
		{
			Page(Ui::Tags::home);
			CntHome();

			break;
		}
		case Ui::Tags::srch: 
		{	
			if(command_.Has(Ui::Tags::show))
			{
				Page(Tags::srchinfo);
				CntSearchResults();
			}
			else 
			{
				Page(Ui::Tags::srch);

				if(command_.Has(Ui::Tags::dn)) 
					CntCreateSearch();
				else if(command_.Has(Ui::Tags::stop))
					System::GetSearchMgr()->Get(command_.Get<ID>(Ui::Tags::stop))->SetRate(0); 
				else if(command_.Has(Ui::Tags::resume))
					System::GetSearchMgr()->Get(command_.Get<ID>(Ui::Tags::resume))->SetRate(Search::Manager::BaseRate()); 
				else if(command_.Has(Ui::Tags::del))
				{
					SearcherPtr p = System::GetSearchMgr()->Get(command_.Get<ID>(Ui::Tags::del));
					System::GetSearchMgr()->Detach(p);
				}

				CntShowSearches();
			}

			break;
		}
		case Ui::Tags::settings:
		{
			if(command_.HasValues())
			{	
				Page(Ui::Tags::status);
				CntMergeSettings();
				System::UpdateSettings();
			}
			else
			{
				Page(Tags::settings);
				CntShowSettings();
			}
			break;
		}
		case Ui::Tags::upl:
		{
			Page(Tags::upl);
			if(command_.Has(Ui::Tags::purge)) System::GetHistory()->ClearUploads();
			CntShowAllUploads();
			break;
		}
		case Ui::Tags::dl:
		{
			if(command_.Has(Ui::Tags::purge)) System::GetHistory()->ClearDownloads();
			if(command_.Has(Ui::Tags::expand))
			{
				Page(Tags::dl);
				GetPrefs().expandDownloads = command_.Get<std::string>(Ui::Tags::expand) == "yes";
			}
			else if(command_.Has(Ui::Tags::selected))
			{
				Page(Tags::dl);
				CntSelectedDownloads();
			}
			else  if(command_.Has(Ui::Tags::link))
			{
				Page(Tags::status);
				CntCreateDownloadFromLink();
			}
			else if(command_.Has(Ui::Tags::srch) && command_.Has(Ui::Tags::id))
			{
				Page(Tags::status);
				CntCreateDownload();
			}
			else if(command_.Has(Ui::Tags::info))
			{
				Page(Tags::dlinfo);
				CntShowDownload();
			}
			else 
			{
				Page(Tags::dl);
				if(command_.Has(Ui::Tags::del))
					System::GetDownloadMgr()->Get(command_.Get<ID>(Ui::Tags::del))->Cancel();
				else if(command_.Has(Ui::Tags::stop))
					System::GetDownloadMgr()->Get(command_.Get<ID>(Ui::Tags::stop))->Pause();
				else if(command_.Has(Ui::Tags::resume))
					System::GetDownloadMgr()->Get(command_.Get<ID>(Ui::Tags::resume))->Resume();
				else if(command_.Has(Ui::Tags::priority))
					System::GetDownloadMgr()->Get(command_.Get<ID>(Ui::Tags::id))->Priority(command_.Get<unsigned>(Ui::Tags::priority));
				System::GetDownloadMgr()->Update();

				CntShowAllDownloads();
			}
			break;
		}
		case Ui::Tags::incoming:
		{
			Page(Tags::incoming);
			if(command_.Has(Ui::Tags::del))
			{
				const std::string name = Url::Decode(command_.Get<std::string>(Ui::Tags::del));

				if(name.find("/") != std::string::npos) 
					throw std::runtime_error("Not a file name");

				System::LogBas() << "Removing completed file " << name << std::endl; 
				Unlink(System::GetSettings()->general.complete + "/" + name);
			}

			CntShowIncoming();

			break;
		}
		case Ui::Tags::log:
		{
			Page(Tags::log);
			CntShowLog();
			break;
		}
		case Ui::Tags::share:
		{
			Page(Tags::share);
			CntShowShare();
			break;
		}
	}
}

void WebuiControl::CntMergeSettings()
{
	for(Ui::Command::Iterator i = command_.Begin(); i != command_.End(); ++i)
	{
		try{ System::GetSettings()->Set(i->first, i->second); }
		catch(std::exception& e)
		{
			System::LogBas() << "Option error: " << e.what() << std::endl;
		}
	}

	Put("status", "Settings applied");
}

void WebuiControl::CntSelectedDownloads()
{
	enum SCom {UNKNOWN, REMOVE, PAUSE, RESUME, PLOW, PBELOW, PNORMAL, PABOVE, PHIGH} scom;
	scom = UNKNOWN;
	
	const std::string selstr = command_.Get<std::string>(Ui::Tags::selected);

	if(selstr == "remove") scom = REMOVE;
	else if(selstr == "pause") scom = PAUSE;
	else if(selstr == "resume") scom = RESUME;
	else if(selstr == "priorityl") scom = PLOW;
	else if(selstr == "priorityb") scom = PBELOW;
	else if(selstr == "priorityn") scom = PNORMAL;
	else if(selstr == "prioritya") scom = PABOVE;
	else if(selstr == "priorityh") scom = PHIGH;

	if(scom == UNKNOWN) 
		throw std::runtime_error("Unknown command for selected downloads");

	std::vector<DownloadPtr> downloads;
	System::GetDownloadMgr()->Dump(std::back_inserter(downloads));
	foreach(DownloadPtr p, downloads)
	{
		const std::string item = "sel" + lexical_cast<std::string>(p->GetId());
		if(!command_.Has(StaticName::FromString(item))) continue;

		switch (scom)
		{
			case REMOVE: p->Cancel(); break;
			case RESUME: p->Resume(); break;
			case PAUSE: p->Pause(); break;
			case PLOW: p->Priority(Download::LOW); break;
			case PBELOW: p->Priority(Download::BELOW); break;
			case PNORMAL: p->Priority(Download::NORMAL); break;
			case PABOVE: p->Priority(Download::ABOVE); break;
			case PHIGH: p->Priority(Download::HIGH); break;
			default: continue;
		}
	}

	System::GetDownloadMgr()->Update();
}

void WebuiControl::CntShowShare()
{

	typedef std::vector<Share::ManagedInfo> ShareVec;
	ShareVec v;
	System::GetShareMgr()->Dump(std::back_inserter(v));

	const uint perPage = System::GetSettings()->webui.pageLines;
	const uint pagen = command_.Has(Ui::Tags::page) ? command_.Get<uint>(Ui::Tags::page) - 1 : 0;
	const uint totalPages = CountPages(v.size(), perPage);

	for(uint i = 1; i <= totalPages; ++i) Put("pagen", i);
	Put("page_cur", pagen + 1);
	Put("page_total", totalPages);

	Ui::Comparers::SharedFiles comparer(GetPrefs().sorting.Option(Ui::Tags::sfl));
	std::sort(v.begin(), v.end(), comparer);

	PagedOutput(pagen, v.begin(), v.end(), ShareWriter(page_));
	
	/*
	foreach(Share::ManagedInfo inf, v)
	{
		Put("name", inf.Name());
		Put("path", inf.Path());
		Put("size", Ui::HumanReadable(inf.Size()));
		Put("type", Ui::FileType(inf.Name()));
		Put("hits", inf.Hits());
		Put("requests", inf.Requests());
	}
	*/

	Put("total_files", v.size());
}

void WebuiControl::CntShowLog()
{

	const unsigned MAX = 1000;
	const std::string log = System::LogFileName();

	std::ifstream f(log.c_str());
	if(!f.good()) throw std::runtime_error("Couldn't open log file: " + log);

	std::string s;
	std::list<std::string> lines;
	while(std::getline(f, s))
	{
		lines.push_back(s);
		if(lines.size() > MAX) lines.pop_front();
	}

	std::reverse(lines.begin(), lines.end());

	foreach(const std::string& msg, lines)
	{
		std::stringstream ss(msg);
		int level;
		std::string timestamp;
		std::string message;
		ss >> level >> timestamp;
		std::getline(ss, message);
		Put("level", level);
		Put("line", timestamp + " " + message);
	}
}

void WebuiControl::CntShowIncoming()
{
	std::vector<std::string> paths;

	ScanDirectory(System::GetSettings()->general.complete, std::back_inserter(paths), FileSystem::REGULAR_FILE, FileSystem::NOT_FOLLOW);

	unsigned n = 0;
	foreach(const std::string& s, paths)
	{
		++n;
		Put("name", Ui::AdaptString(FileName(s)));
		Put("size", Ui::HumanReadable(FileSize(s)));
		Put("type", Ui::FileType(s));
	}

	Put("total_files", n);
}

void WebuiControl::CntCreateDownloadFromLink()
{
	const std::string link = Url::Decode(command_.Get<std::string>(Ui::Tags::link));
	if(starts_with(link, "ed2k://")) CntCreateDownloadFromEd2k(link);
	else if(starts_with(link, "magnet:?")) CntCreateDownloadFromMagnet(link);
	else Put("status", "Link is not supported");
}

void WebuiControl::CntCreateDownloadFromMagnet(const std::string& link)
{
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Should be end here....");
	std::stringstream s(link.substr(8));
	std::string line;
	DownloadRequest req;
	std::map<Ip::Endpoint, std::string> sources;
	while(std::getline(s, line, '&'))
	{
		line = Url::Decode(line);
		std::cout << line << std::endl;
		if(starts_with(line, "xt=urn:ed2k:")) req.ed2k.FromString(line.substr(12));
		else if(starts_with(line, "xt=urn:ed2khash:")) req.ed2k.FromString(line.substr(16));
		else if(starts_with(line, "xt=urn:bitprint:"))
		{
			Hashes::BP bp;
			bp.FromString(line.substr(16)); 
			bp.Split(req.sha1, req.ttr);
		}
		else if(starts_with(line, "xt=urn:sha1:")) req.sha1.FromString(line.substr(12));
		else if(starts_with(line, "xt=urn:md5:")) req.md5.FromString(line.substr(11));
		else if(starts_with(line, "xl=")) req.size = lexical_cast<file_offset_t>(line.substr(3));
		else if(starts_with(line, "dn=")) req.name = line.substr(3);
		else if(starts_with(line, "xs="))
		{
			try
			{
				std::size_t pos = line.find("http://");
				line.erase(0, pos + 7);
				pos = line.find("/");
				const std::string ep = line.substr(0, pos);
				const std::string uri = line.substr(pos);
				Ip::Endpoint endpoint;
				endpoint.FromString(ep);
				sources[endpoint] = uri;
			}
			catch(std::exception& e)
			{
				System::LogBas() << "Couldn't parse magnet xs tag: " << e.what() << std::endl;
			}
		}
	}

	if(req.sha1.Empty() && req.ed2k.Empty() && req.ttr.Empty() && req.md5.Empty()) throw std::runtime_error("Link has no hash");

	DownloadPtr pDownload;
	try { pDownload = System::GetDownloadMgr()->Create(req); }
	catch (std::exception& err)
	{
		Put("status", "Can't create download. Reason: " + std::string(err.what()));
		return; 
	}
	
	for(std::map<Ip::Endpoint, std::string>::iterator i = sources.begin(); i != sources.end(); ++i)
	{
		SourceHost src;
		src.endpoint = i->first;
		src.uri = i->second;
		pDownload->AddSource(src);
	}

	Put("status", "Download created");
}


void WebuiControl::CntCreateDownloadFromEd2k(const std::string& link)
{
	std::stringstream s(link);
	
	std::string part;
	while(std::getline(s, part, '|') && part != "file");

	DownloadRequest req;

	if(!(std::getline(s, req.name, '|'))) throw std::runtime_error("Can't parse file name");

	std::getline(s, part, '|'); 
	req.size = lexical_cast<file_offset_t>(part);

	std::getline(s, part, '|');
	req.ed2k.FromString(part);

	DownloadPtr pDownload;
	try { pDownload = System::GetDownloadMgr()->Create(req); }
	catch (std::exception& err)
	{
		Put("status", "Can't create download. Reason: " + std::string(err.what()));
		return; 
	}
	
	Put("status", "Download created");
}

void WebuiControl::CntShowSettings()
{
	Settings* p = System::GetSettings();
	Put("username", p->general.userName);
	Put("webuiport", p->webui.port);
	Put("wuipagelines", p->webui.pageLines);
	Put("logsize", p->general.logSize);
	
	std::string s;
	foreach(IpSegment seg, p->general.uisegments)
	{
		s += seg.First().ToString();
		if(seg.First() != seg.Last())
			s += "-" + seg.Last().ToString();
		s += "; ";
	}
	/*
	IpRanges::iterator ir;
	for(ir = settings.net.remote_ui_ranges.begin(); 
			  ir != settings.net.remote_ui_ranges.end(); ir++) {
		if(ir != settings.net.remote_ui_ranges.begin()) s += ", ";
		s += IpToStr(ir->first);
		if(ir->second) s += "-" + IpToStr(ir->second);
	}
	*/
	Put("remoteranges", s);
	
	Put("telnetport", p->telnet.port);
	Put("webuistyle", p->webui.style);
	Put("listenport", p->net.listenPort);
	switch(p->net.directAble)
	{
		case Settings::YES: s = "yes"; break;
		case Settings::NO: s = "no"; break;
		default: s = "auto"; break;
	}
	Put("directable", s);
	Put("max_bw_in_kb", p->net.maxBwIn / 1024);
	Put("max_bw_out_kb", p->net.maxBwOut / 1024);
	
	s.clear();
	foreach(const std::string& path, p->share.folders)
		s += path + "; ";
	Put("share", s);
	
	Put("complete", p->general.complete);
	Put("incomplete", p->general.incomplete);
	Put("maxhubs", p->net.maxHubs);
	Put("uploadsperhost", p->net.uploadsPerHost);
	Put("maxuploads", p->net.maxUploads);
	Put("queuesize", p->net.queueSize);
	Put("maxdownloads", p->net.maxDownloads);
	Put("maxfiles", p->net.maxFiles);
	Put("hashrate", p->share.hashRate);
}

void WebuiControl::CntShowAllUploads()
{
	std::vector<Uploader::Ptr> ptrs;
	System::GetUploadMgr()->Dump(std::back_inserter(ptrs));
	Put("total_uploads", ptrs.size());
	for(unsigned i = 0; i < ptrs.size(); ++i)
	{
		Uploader::Ptr p = ptrs[i];
		Share::ManagedInfo inf = p->GetFileInfo();
		if(!inf.Lock()) continue;

		if(p->Status() != Uploader::QUEUED)
		{
			Put("a_name", Ui::AdaptString(inf.Name()));
			Put("a_size", Ui::HumanReadable(p->Sent()));
			Put("a_rate", p->AvrRate() / 1024);
			Put("a_address", p->Address().address.ToString());
			Put("a_nick", Url::Decode(p->Nick()));
			Put("a_type", Ui::FileType(inf.Name()));
			Put("a_client", Ui::AdaptString(p->ClientName()));
		}
		else
		{
			Put("q_name", Ui::AdaptString(inf.Name()));
			Put("q_address", p->Address().address.ToString());
			Put("q_nick", Url::Decode(p->Nick()));
			Put("q_type", Ui::FileType(inf.Name()));
			Put("q_client", Ui::AdaptString(p->ClientName()));
		}
	}

	file_offset_t sz = 0;
	unsigned n = 0;
	foreach(const History::Upload& u, System::GetHistory()->GetUploads())
	{
		sz += u.sent;
		++n;

		Put("h_name", Ui::AdaptString(FileName(u.path)));
		Put("h_size", Ui::HumanReadable(u.sent));
		Put("h_address", u.addr.address.ToString());
		Put("h_nick", Url::Decode(u.nick));
		Put("h_type", Ui::FileType(u.path));
		Put("h_client", Ui::AdaptString(u.client));
	}

	Put("total_history", n);
	Put("total_size", Ui::HumanReadable(sz));
}

void WebuiControl::CntShowDownload()
{
	DownloadPtr pdl = System::GetDownloadMgr()->Get(command_.Get<ID>(Ui::Tags::info));
	Share::FileInfo inf = pdl->GetFileInfo().Info();
	
	Put("id", inf.partial);
	Put("name", inf.name);
	Put("path", inf.path);
	Put("size", inf.size);
	Put("hrsize", Ui::HumanReadable(inf.size));
	Put("progress", Ui::Progress(pdl->GetFileMap()->CountBytes(Integrity::FULL | Integrity::VALID), pdl->GetFileMap()->CountBytes()));
	Put("downloaders", pdl->CountSources());
	Put("rate", Ui::HumanReadable(pdl->AvrRate()));
	Put("sha1", inf.sha1.Empty() ? "n/a" : inf.sha1.ToString());
	Put("ttr", inf.ttr.Empty() ? "n/a" : inf.ttr.ToString());
	Put("md5", inf.md5.Empty() ? "n/a" : inf.md5.ToString());
	Put("ed2k", inf.ed2k.Empty() ? "n/a" : inf.ed2k.ToString());
	Put("bp", (inf.sha1.Empty() || inf.ttr.Empty()) ? "n/a" : inf.sha1.ToString() + "." + inf.ttr.ToString());
	Put("tiger_min", inf.tigerTree.Empty() ? "n/a" : boost::lexical_cast<std::string>(inf.tigerTree.MinimalRange()));
	

	std::vector<FileRangeEx> ranges;
	pdl->GetFileMap()->Dump(std::back_inserter(ranges));
	foreach(FileRangeEx r, ranges)
	{
		uint percent = 100 * double(double(r.Length()) / inf.size);
		Put("chunk_size", percent);
		switch(r.Status())
		{
			case Integrity::EMPTY: Put("chunk_status", "empty"); break;
			case Integrity::FULL: Put("chunk_status", "full"); break;
			case Integrity::VALID: Put("chunk_status", "valid"); break;
			case Integrity::INVALID: Put("chunk_status", "invalid"); break;
			case Integrity::ACTIVE: Put("chunk_status", "active"); break;
			default: Put("chunk_status", "mixed");
		}
	}

	std::map<Integrity::Type, std::pair<uint, file_offset_t> > table;
	foreach(FileRangeEx r, ranges)
	{
		table[r.Status()].first++;
		table[r.Status()].second += r.Length();
	}

	Put("parts_empty", table[Integrity::EMPTY].first);
	Put("parts_empty_p", Ui::Progress(table[Integrity::EMPTY].second, inf.size));
	Put("parts_full", table[Integrity::FULL].first);
	Put("parts_full_p", Ui::Progress(table[Integrity::FULL].second, inf.size));
	Put("parts_valid", table[Integrity::VALID].first);
	Put("parts_valid_p", Ui::Progress(table[Integrity::VALID].second, inf.size));
	Put("parts_invalid", table[Integrity::INVALID].first);
	Put("parts_invalid_p", Ui::Progress(table[Integrity::INVALID].second, inf.size));
	Put("parts_active", table[Integrity::ACTIVE].first);
	Put("parts_active_p", Ui::Progress(table[Integrity::ACTIVE].second, inf.size));

	std::vector<SourceHostEx> sources;
	pdl->CopySources(std::back_inserter(sources));
	uint nsource = 0;
	foreach(const SourceHostEx& host, sources)
	{
		++nsource;
		Put("source_number", nsource);
		Put("source_endpoint", host.endpoint.ToString());
		Put("source_connections", host.connections);
		Put("source_max", host.maxConnections);
		const std::time_t after = 
			System::Now() < host.connectAfter ? 
			host.connectAfter - System::Now() : 0;
		Put("source_after", after);
		Put("source_nick", Url::Encode(host.nick));
	}
}

void WebuiControl::CntShowAllDownloads()
{

	Ui::Template tplDlrs(Tpl("downloaders"));

	std::vector<DownloadPtr> dls;
	System::GetDownloadMgr()->Dump(std::back_inserter(dls));
	Ui::Comparers::Download comparer(GetPrefs().sorting.Option(Ui::Tags::adl));
	std::sort(dls.begin(), dls.end(), comparer);
	//Ui::CreateOrderedSRGroups(results.begin(), results.end(), groups, GetPrefs().sorting.Option(Ui::Tags::sres));

	Put("expand_downloads", GetPrefs().expandDownloads ? "yes" : "no");

	Put("total_downloads", dls.size());
	for(uint i = 0; i < dls.size(); ++i)
	{
		DownloadPtr p = dls[i];
		Share::FileInfo inf = p->GetFileInfo().Info();

		Put("download_name", Ui::AdaptString(inf.name));
		Put("download_size", Ui::HumanReadable(inf.size));
		Put("download_rate", Ui::HumanReadable(p->AvrRate()));
		Put("sources", p->CountSources());
		Put("active_sources", p->CountDownloaders());
		Put("progress", Ui::Progress(p->GetFileMap()->CountBytes(Integrity::FULL | Integrity::VALID), p->GetFileMap()->CountBytes()));
		Put("download_id", inf.partial);
		Put("priority", int(p->Priority()));
		Put("dl_paused", p->Status() == Download::PAUSE);

		switch(p->Status())
		{
			case Download::TRANSFER: Put("status", "transfer"); break; 
			case Download::HASH: Put("status", "hash"); break; 
			case Download::MOVE: Put("status", "move"); break; 
			case Download::PAUSE: Put("status", "paused"); break; 
			case Download::STOP: Put("status", "queued"); break; 
			case Download::ERROR: Put("status", "error"); break; 
			default: assert(!"can't be here");
		}

		if( !GetPrefs().expandDownloads ) 
		{
			Put("downloaders", "");
			continue;
		}

		tplDlrs.ClearValues();
		typedef boost::shared_ptr <Downloader> DownloaderPtr;
		std::vector< DownloaderPtr > downloaders;
		p->Dump(std::back_inserter(downloaders));

		foreach(DownloaderPtr pDlr, downloaders)
		{
			Ui::Template& t = tplDlrs;
			t.Put("address", pDlr->HostInfo()->endpoint.ToString());
			t.Put("pushed", pDlr->Pushed());
			
			switch (pDlr->Status()) 
			{
				case Downloader::IDLE: t.Put("status", "idle"); break;
				case Downloader::ERROR: t.Put("status", "error"); break;
				case Downloader::CONNECT: t.Put("status", "connect"); break;
				case Downloader::TRANSFER: t.Put("status", pDlr->Task() == Downloader::FETCH_FILE ? "file" : "tiger"); break;
				case Downloader::CHAT: t.Put("status", "talking"); break;
				case Downloader::QUEUE: t.Put("status", "queued"); break;
				case Downloader::CLOSED: t.Put("status", "closed"); break;
			};
				
			t.Put("left", Ui::RankedNumber(pDlr->GetRange().First(), "&nbsp;"));
			t.Put("right", Ui::RankedNumber(pDlr->GetRange().Last(), "&nbsp;"));
			t.Put("length", Ui::RankedNumber(pDlr->GetRange().Length(), "&nbsp;"));
			t.Put("total", Ui::HumanReadable(pDlr->Received()));
			t.Put("qnum", pDlr->QueuePos());
			t.Put("qsize", pDlr->QueueTotal());;
			t.Put("rate", Ui::HumanReadable(pDlr->AvrRate()));
		}

		Put("downloaders", tplDlrs);
	}

	foreach(const History::Download& d, System::GetHistory()->GetDownloads())
	{
		Put("recently_name", Ui::AdaptString(FileName(d.path)));
		Put("recently_type", Ui::FileType(d.path));
		Put("recently_size", Ui::HumanReadable(d.size));
	}
}

void WebuiControl::CntCreateDownload()
{
	SearcherPtr p;
	ID id;
	try
	{
		p = System::GetSearchMgr()->Get(command_.Get<ID>(Ui::Tags::srch));
		id = command_.Get<ID>(Ui::Tags::id);
	}
	catch(Search::Manager::Error&)
	{
		Put("status", "Bad search");
		return;
	}

	Search::Result r;
	try { r = p->GetResult(id); }
	catch (std::runtime_error&)
	{
		Put("status", "Bad result");
		return;
	}

	DownloadRequest req;
	req.sha1 = r.hit.sha1;
	req.ttr = r.hit.ttr;
	req.md5 = r.hit.md5;
	req.ed2k = r.hit.ed2k;
	req.size = r.hit.size;
	req.name = r.hit.dn;

	DownloadPtr pDownload;
	try { pDownload = System::GetDownloadMgr()->Create(req); }
	catch (std::exception& err)
	{
		Put("status", "Can't create download. Reason: " + std::string(err.what()));
		return; 
	}
	

	std::vector<Search::Result> results;
	p->DumpResults(std::back_inserter(results));
	for(std::size_t i = 0; i < results.size(); ++i)
	{
		const Search::Hit& tmp = results[i].hit;
		if(
			(!tmp.sha1.Empty() && tmp.sha1 == req.sha1) || 
			(!tmp.ttr.Empty() && tmp.ttr == req.ttr) || 
			(!tmp.md5.Empty() && tmp.md5 == req.md5) || 
			(!tmp.ed2k.Empty() && tmp.ed2k == req.ed2k)
		  )
		{
			pDownload->AddSource(results[i].host);
		}
	}

	Put("status", "Download created");
}

void WebuiControl::CntHome()
{

	Put("full_name", Ident::fullname);
	Put("total_downloads", System::GetDownloadMgr()->Count());
	Put("total_uploads", System::GetUploadMgr()->Count());
	Put("total_searches", System::GetSearchMgr()->CountUserSearches());
	Put("hr_avr_bw_in", Ui::HumanReadable(System::GetTraffIn()->Usage()));
	Put("hr_avr_bw_out", Ui::HumanReadable(System::GetTraffOut()->Usage()));
	Put("firewalled", (System::IsFirewalled() ? "yes" : "no"));

	int counter = 0;

	typedef boost::shared_ptr<G2Node> NodePtr;
	typedef std::vector < NodePtr > Hubs;
	Hubs hubs;
	System::GetNetwork()->CopyHubs(std::back_inserter(hubs));
	foreach(NodePtr p, hubs)
	{
		Put("num", ++counter);
		Put("address", p->Info().endpoint.ToString());
		Put("status", "connected");
	}

	typedef boost::shared_ptr<G2Handshake> HandshakePtr;
	typedef std::vector < HandshakePtr > Handshakes;
	Handshakes handshakes;
	System::GetNetwork()->CopyHandshakes(std::back_inserter(handshakes));
	foreach(HandshakePtr p, handshakes)
	{
		Put("num", ++counter);
		Put("address", p->GetEndpoint().ToString());
		if(p->Connecting())
			Put("status", "connecting");
		else
			Put("status", "handshaking");
	}

	const UpdatesWatch* pUpdate = System::GetUpdatesWatch();
	if(pUpdate->NewVersion())
	{
		Put("version", pUpdate->Tag());
		Put("vlink", pUpdate->Link());
		std::string comments = pUpdate->Text();
		boost::replace_all(comments, "\n", "<br>");
		Put("vcomments", comments);
	}
}

void WebuiControl::CntShowSearches()
{
	typedef std::list<SearcherPtr> SearchersType;
	SearchersType searchers;
	System::GetSearchMgr()->DumpSearchers(std::back_inserter(searchers));
	Put("total_searches", System::GetSearchMgr()->CountUserSearches());

	for(SearchersType::iterator i = searchers.begin(); i != searchers.end(); ++i)
	{
		SearcherPtr p = *i;
		if(p->GetID() == INVALID_ID) continue;

		Search::Criteria crit = p->GetCriteria();
		Put("id", p->GetID());
		std::stringstream s;
		s << NameFromCriteria(crit);
		if(crit.min) 
			s << ", Min: " << Ui::HumanReadable(crit.min);
		if(crit.max) 
			s << ", Max: " << Ui::HumanReadable(crit.max);
		Put("criteria", s.str());

		std::string exts;
		foreach(const std::string& s, crit.extensions) exts += s + " ";
		Put("extensions", exts);

		Put("hits", p->CountResults());
		Put("active", p->Rate());
		Put("endless", "1");
	}
}

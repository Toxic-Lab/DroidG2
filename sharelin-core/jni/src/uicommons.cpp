/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009  Andrey Stroganov

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

#include "uicommons.hpp"
#include <sstream>
#include "hashes.hpp"
#include <boost/lexical_cast.hpp>
#include <cctype>
#include "download.hpp"

using boost::lexical_cast;

namespace Ui {

//typedef std::vector<srch::ResultList::iterator> ResultIterators;

std::string HumanReadable(int64 x)
{
	std::stringstream s;
	if (x < 1000) s << x << " B";
	else if (x < 1024*1024) s << x / 1024 << " KB";
	else if (x < 1024*1024*1024) s << x / (1024*1024) << " MB";
	else s << x / (1024*1024*1024) << " GB";
	return s.str();
}

bool operator< (const TempRankName& r1, const TempRankName& r2)
{
	return r1.counter > r2.counter;
}

uint64 GetSizeValue(const std::string& s) 
{
	std::size_t i = 0;
	while(i < s.size() && std::isdigit(s[i])) ++i;

	uint64 val = boost::lexical_cast<uint64>(s.substr(0, i));

	while(i < s.size() && s[i] == ' ') ++i;
	if(i == s.size()) return val;
	assert(i < s.size());

	if(s[i] == 'k' || s[i] == 'K') return val * 1024;
	if(s[i] == 'm' || s[i] == 'M') return val * 1024 * 1024;
	if(s[i] == 'g' || s[i] == 'G') return val * 1024 * 1024 * 1024;

	throw std::runtime_error("Bad size suffix");

	return val;
}

std::string RankedNumber(uint64 x, const std::string& separator)
{
	std::string s = lexical_cast<std::string>(x);
	std::string ss;
	for(int i = s.size() - 1, j = 1; i >= 0; --i, ++j) 
	{
		ss.insert(0, 1, s[i]);
		if(j % 3 == 0 && i > 0)
			ss = separator + ss;
	}
	return ss;	
}

std::string AdaptString(std::string s)
{
 	const uint max = 80;
 	//we want to ensure that there's a possibility (a space) in the line in every max sequence

 	uint count = 0;
 	uint not_alnum = 0;
 	for(uint i = 0; i < s.size(); i++) 
	{
 		if(s[i] == ' ') count = 0;
 		else count++;

 		if(!isalnum(s[i])) not_alnum = i;

 		if(count == max) 
		{
 			//let's add a space
			if(i - not_alnum >= max - 1 || not_alnum == 0) //not alnum is too far
 				not_alnum = i;
 			count = i - not_alnum;

 			if(not_alnum + 1 == s.size()) break;

 			s.insert(not_alnum + 1, " ");
 			not_alnum = 0;
 		}
 	}
 	return s;
}

uint DownloadProgress(DownloadPtr p)
{
	return Progress(p->GetFileMap()->CountBytes(Integrity::FULL | Integrity::VALID), p->GetFileMap()->CountBytes());
}

uint Progress(uint64 n, uint64 total)
{
	return total == 0 ? 0 : 100 * n / total;
}

std::string FileType(const std::string& name)
{
 	//std::string::size_type pos = name.find_last_of(".");
 	//if(pos >= name.size() - 1) return "unknown";

 	//std::string ext = boost::to_lower_copy(name.substr(pos + 1));
	return StaticName::ToString(Media::ResolveByName(name));
}
/*

	const char video[] = " avi mp4 mpeg mpg ogm ";
 	const char audio[] = " mp3 wav mid ogg ";
 	const char archive[] = " rar zip 7z cab ";
 	const char virus[] = " exe com ";
 	const char iso[] = " iso nrg bin ";
 	const char document[] = " pdf doc txt chm docx odt rtf ";
 	const char picture[] = " bmp psd gif png jpg jpeg xcf ";

 	if(ext == "tmp") return "tmp";
 	else if(std::strstr(video, ext.c_str()) != NULL)
 		return "video";
 	else if(std::strstr(audio, ext.c_str()) != NULL)
		return "audio";
 	else if(std::strstr(archive, ext.c_str()) != NULL)
 		return "archive";
 	else if(std::strstr(virus, ext.c_str()) != NULL)
 		return "virus"; //most likely =)
 	else if(std::strstr(iso, ext.c_str()) != NULL)
 		return "iso";
 	else if(std::strstr(document, ext.c_str()) != NULL)
 		return "doc";
 	else if(std::strstr(picture, ext.c_str()) != NULL)
		return "picture";
 	else
 		return "unknown";
}
*/

} //namespace Ui 

/*
#include "sharemanager.h"
#include <boost/bind.hpp>
#include <functional>
#include <algorithm>
#include "misc.h"
#include "searchmanager.h"
#include "downloadmanager.h"
#include <stdexcept>
#include "link.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "settings.h"
#include <stdio.h>

using namespace ui;

extern ShareManager share_manager;
extern SearchManager search_manager;
extern DownloadManager download_manager;
extern Settings settings;
extern StatisticsManager statisticsManager;

struct SF_to_SFStats
{
	SharedFileStats operator()(const SharedFile& sf)
	{
		SharedFileStats f;
		f.name = sf.name;
		f.size = sf.size;
		f.hits = sf.hits;
		f.requests = sf.requests;
		return f;
	}
};


struct DLSortInfo 
{
	DLSortInfo() {}
	DLSortInfo(ID id_, const DownloadInfo& inf)
	{
		id = id_;
		done = ui::Progress(inf.file_map->BytesInParts(FP_VALID | FP_FULL), inf.file_size);
		name = inf.file_name;
		size = inf.file_size;
		sources = inf.downloaders.size();
		rate = inf.avr_in;
		priority = inf.priority;
	}

	ID id;
	uint done;
	std::string name;
	uint64 size;
	uint sources;
	uint rate;
	byte priority;

};



ui::SFStatsContainer ui::GetOrderedSFStatistics(const SortOptions& sopt) 
{
	SFStatsContainer v;
	share_manager.Dump(std::back_inserter(v), SF_to_SFStats());
	if(ToLower(sopt.sortby) == "size")
		std::sort(v.begin(), v.end(), boost::bind(std::less<uint64>(), boost::bind(&SharedFileStats::size, _1), boost::bind(&SharedFileStats::size, _2)));
	else if(ToLower(sopt.sortby) == "hits")
		std::sort(v.begin(), v.end(), boost::bind(std::less<uint>(), boost::bind(&SharedFileStats::hits, _1), boost::bind(&SharedFileStats::hits, _2)));
	else if(ToLower(sopt.sortby) == "requests")
		std::sort(v.begin(), v.end(), boost::bind(std::less<uint>(), boost::bind(&SharedFileStats::requests, _1), boost::bind(&SharedFileStats::requests, _2)));
	else
		std::sort(v.begin(), v.end(), boost::bind(std::less<std::string>(), boost::bind(&SharedFileStats::name, _1), boost::bind(&SharedFileStats::name, _2)));

	if(!sopt.forward)
		std::reverse(v.begin(), v.end());

	return v;
}

ui::IDList ui::GetOrderedDownloadsIDs(const SortOptions& sopt)
{
	typedef std::vector<DLSortInfo> InfoVector;

	InfoVector v;

	IDList ids;
	download_manager.DumpIDs(std::back_inserter(ids));
	for(IDList::iterator i = ids.begin(); i != ids.end(); ++i)
	{
		DownloadInfo inf;
		if(download_manager.GetDownloadInfo(*i, inf))
			v.push_back(DLSortInfo(*i, inf));
	}
	ids.clear();

	if(ToLower(sopt.sortby) == "done")
		std::sort(v.begin(), v.end(), boost::bind(std::less<uint>(), boost::bind(&DLSortInfo::done, _1), boost::bind(&DLSortInfo::done, _2)));
	else if(ToLower(sopt.sortby) == "size")
		std::sort(v.begin(), v.end(), boost::bind(std::less<uint64>(), boost::bind(&DLSortInfo::size, _1), boost::bind(&DLSortInfo::size, _2)));
	else if(ToLower(sopt.sortby) == "rate")
		std::sort(v.begin(), v.end(), boost::bind(std::less<uint>(), boost::bind(&DLSortInfo::rate, _1), boost::bind(&DLSortInfo::rate, _2)));
	else if(ToLower(sopt.sortby) == "sources")
		std::sort(v.begin(), v.end(), boost::bind(std::less<uint>(), boost::bind(&DLSortInfo::sources, _1), boost::bind(&DLSortInfo::sources, _2)));
	else if(ToLower(sopt.sortby) == "name") 
		std::sort(v.begin(), v.end(), boost::bind(std::less<std::string>(), boost::bind(&DLSortInfo::name, _1), boost::bind(&DLSortInfo::name, _2)));
	else if(ToLower(sopt.sortby) == "priority") 
		std::sort(v.begin(), v.end(), boost::bind(std::less<byte>(), boost::bind(&DLSortInfo::priority, _1), boost::bind(&DLSortInfo::priority, _2)));

	std::transform(v.begin(), v.end(), std::back_inserter(ids), boost::bind(&DLSortInfo::id, _1));

	if(!sopt.forward)
		std::reverse(ids.begin(), ids.end());

	return ids;
}







ID ui::DownloadWithLink(const std::string& link)
{
	DownloadRequest rq;

	if(!TranslateLink(link, rq))
		throw std::runtime_error("Bad link format");

	ID id = download_manager.CreateDownload(rq);
	if(id == 0) 
		throw std::runtime_error("Invalid parameters (perhaps we have already this download) or there is no space on disk");
	return id;
}

ID ui::DownloadWithSearch(ID sid, ID rid)
{
	Search::Ptr pSearch = search_manager.GetSearch(sid);
	if(!pSearch)
		throw std::runtime_error("Bad search id");

	srch::ResultList results = pSearch->GetResults();
	for(srch::ResultList::iterator i = results.begin(); i != results.end(); ++i)
		if(i->id == rid)
		{
			srch::Hit hit = i->hit;
			DownloadRequest rq;
			rq.base_name = hit.dn;
			rq.sha1 = hit.sha1;
			rq.md5 = hit.md5;
			rq.ttr = hit.ttr;
			rq.ed2k = hit.ed2k;
			rq.bp = hit.bp;
			rq.size = hit.size;
			ID id = download_manager.CreateDownload(rq);
			if(id == 0) 
				throw std::runtime_error("Invalid parameters (perhaps we have already this download) or there is no space on disk");
			return id;
		}
	throw std::runtime_error("Bad result id");
}

ui::IncomingFilesType ui::GetOrderedIncomingFiles(const ui::SortOptions& sopt)
{
	ui::IncomingFilesType files;
	std::set<std::string> names;
	FindFiles(settings.general.complete_folder, names, false, 0);
	for(std::set<std::string>::iterator i = names.begin(); i != names.end(); ++i)
	{
		int fd = open(i->c_str(), O_RDONLY);
		if(fd >= 0) 
		{
			IncomingFile f;
			f.name = RelativeName(*i);
			f.size = lseek(fd, 0, SEEK_END);
			close(fd);
			files.push_back(f);
		}
	}

	if(ToLower(sopt.sortby) == "name")
		std::sort(files.begin(), files.end(), 
				boost::bind(std::less<std::string>(), boost::bind(&IncomingFile::name, _1), boost::bind(&IncomingFile::name, _2)));
	else if(ToLower(sopt.sortby) == "size")
		std::sort(files.begin(), files.end(), boost::bind(std::less<uint64>(), boost::bind(&IncomingFile::size, _1), boost::bind(&IncomingFile::size, _2)));

	if(!sopt.forward)
		std::reverse(files.begin(), files.end());

	return files;
}
*/

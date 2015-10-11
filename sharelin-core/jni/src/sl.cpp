/*
	SL.CPP, an implementation of the JNI interface for Sharelin.
    Copyright (C) 2012  Lai Yiu Ming

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

#define DEFINE_GLOBALS
#include "sl.hpp"
#include <android/log.h>
#include <string>
#include <fstream>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

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

using std::string;

typedef boost::shared_ptr<Search::Searcher> SearcherPtr;

struct toxiclab_search{
	string criteria;
	string hits;
	string active;
	string endless; //always = 1??
	string extensions;
	ID id;
};

struct toxiclab_searchResult{
	string name;
	ID rid;
	ID sid;

	string size;
	string type;
	int sources;
};

struct toxiclab_hub{
	string num;
	string address;
	string status;
};

struct toxiclab_fileRange{
	uint percentage;
	string status;
};

struct toxiclab_sourceHost{
	string source_number;
	string source_endpoint;
	string source_connection;
	string source_max;
	string source_after;
	string source_nick;
};

struct toxiclab_downloadResult{
	ID id;
	string name;
	string path;
	string size;
	string hrsize;
	string progress;
	string downloaders;
	string rate;
	string sha1;
	string ttr;
	string md5;
	string ed2k;
	string bp;
	string tiger_min;

	int range_length;
	toxiclab_fileRange* range;

	int host_length;
	toxiclab_sourceHost* hosts;
};

struct toxiclab_download{
	string download_name;
	string download_size;
	string download_rate;
	uint sources;
	uint active_sources;
	string progress;
	ID download_id;
	int priority;
	bool dl_paused;
	string status;
	string downloaded_byte;
};

struct toxiclab_incoming{
	string name;
	string size;
	string type;
	string parent;
};

int another_main();

std::string toxiclab_NameFromCriteria(const Search::Criteria& criteria)
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

JNIEXPORT void JNICALL Java_org_toxiclab_droidg2_main_start_1sharelin
  (JNIEnv * env , jobject obj, jstring root)
{
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "entering JNI interface.");
	root_directory = env->GetStringUTFChars(root, 0);
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", root_directory);
	another_main();
}

//ext format becareful
void toxiclab_createSearch(string dn, string min, string max, string media, string ext){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_createSearch");
	Search::Criteria criteria;

	if(boost::algorithm::istarts_with(dn, "urn:sha1:"))
		criteria.sha1.FromString(dn.substr(9));
	else if(boost::algorithm::istarts_with(dn, "urn:ttr:"))
		criteria.ttr.FromString(dn.substr(8));
	else if(boost::algorithm::istarts_with(dn, "urn:ed2k:"))
		criteria.ed2k.FromString(dn.substr(9));
	else if(boost::algorithm::istarts_with(dn, "urn:md5:"))
		criteria.md5.FromString(dn.substr(8));
	else criteria.dn = dn;

	if(min.size() != 0)
		criteria.min = atoi(min.c_str());
	if(max.size() != 0)
		criteria.max = atoi(max.c_str());
	if(media.size() != 0)
	{
		if(media != "any")
			criteria.media = StaticName::FromString(media);
	}
	if(ext.size() != 0)
	{
		std::stringstream s(ext);
		std::string ext;
		while(s >> ext)
			criteria.extensions.insert(boost::to_lower_copy(ext));
	}
	System::GetSearchMgr()->Create(criteria, Search::Manager::USER);
}

//return searches.
//allocated memory for searches, please remove them after use.
int toxiclab_showSearches(toxiclab_search*& result){
	//char debug[10];
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_showSearches");
	typedef std::list<SearcherPtr> SearchersType;
	SearchersType searchers;
	System::GetSearchMgr()->DumpSearchers(std::back_inserter(searchers));

	int _tsearch = System::GetSearchMgr()->CountUserSearches();
	result = new toxiclab_search[_tsearch];
	for(int i=0; i<_tsearch; ++i){
		result[i].id = INVALID_ID;
	}

	int cnt = 0;

	for(SearchersType::iterator i = searchers.begin(); i != searchers.end(); ++i)
	{
		SearcherPtr p = *i;
		if(p->GetID() == INVALID_ID) continue;

		Search::Criteria crit = p->GetCriteria();
		result[cnt].id = p->GetID();

		std::stringstream s;
		s << toxiclab_NameFromCriteria(crit);
		if(crit.min)
			s << ", Min: " << Ui::HumanReadable(crit.min);
		if(crit.max)
			s << ", Max: " << Ui::HumanReadable(crit.max);
		result[cnt].criteria = s.str();

		std::string exts;
		foreach(const std::string& s, crit.extensions) exts += s + " ";
		result[cnt].extensions = exts;

		char temp[10];
		sprintf(temp, "%d", p->CountResults());
		result[cnt].hits = temp;
		sprintf(temp, "%d", p->Rate());
		result[cnt].active = temp;
		result[cnt].endless = "1";

		++cnt;
	}

	return _tsearch;
}

//return search result
//allocated memory for search result, delete them after use.
int toxiclab_showSearchResult(ID id, string sortBy, bool forward, toxiclab_searchResult*& result){
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_showSearchResult");
	SearcherPtr pSearcher;
	char debug[50];
	//sprintf(debug, "before getting pSearcher... %d", id);
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	pSearcher = System::GetSearchMgr()->Get(id);
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "after getting pSearcher...");

	typedef std::vector<Search::Result> ResultsContainer;
	ResultsContainer results;
	pSearcher->DumpResults(std::back_inserter(results));

	Ui::SRGroupContainer groups;
	//sorted the result
	Ui::SortOption so;
	if(sortBy == "name")
		so.SortBy(Ui::Tags::name);
	else if(sortBy == "sources")
		so.SortBy(Ui::Tags::sources);
	else if(sortBy == "size")
		so.SortBy(Ui::Tags::size);
	else
		so.SortBy(0);
	so.Forward(forward);

	Ui::CreateOrderedSRGroups(results.begin(), results.end(), groups, so);

	if(results.empty()) return 0;

	int length = groups.size();
	result  = new toxiclab_searchResult[length];
	string fileType;

	for(int i=0; i<length; ++i){
		Ui::SearchResultGroup::RankedNames::const_iterator iName = groups[i].names.begin();
		if(groups[i].names.begin() != groups[i].names.end()){
			result[i].name = Ui::AdaptString(iName->first);
			result[i].rid = iName->second;
			if(fileType.empty())
				fileType = Ui::FileType(iName->first);
			result[i].size = Ui::HumanReadable(groups[i].fileSize);
		}

		result[i].sid = id;
		result[i].type = fileType;
		result[i].sources = groups[i].sources;
	}

	return length;
}

int toxiclab_showHubs(toxiclab_hub*& result){
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_showHubs");
	typedef boost::shared_ptr<G2Node> NodePtr;
	typedef std::vector < NodePtr > Hubs;
	Hubs hubs;
	System::GetNetwork()->CopyHubs(std::back_inserter(hubs));

	int length = hubs.size();
	result = new toxiclab_hub[length];
	int cnt = 0;
	foreach(NodePtr p, hubs)
	{
		result[cnt].num = cnt;
		result[cnt].address = p->Info().endpoint.ToString();
		result[cnt].status = "connected";
		++cnt;
	}

	return length;
}

int toxiclab_showHandshake(toxiclab_hub*& result){
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_showHandshake");
	typedef boost::shared_ptr<G2Handshake> HandshakePtr;
	typedef std::vector < HandshakePtr > Handshakes;
	Handshakes handshakes;
	System::GetNetwork()->CopyHandshakes(std::back_inserter(handshakes));

	int length = handshakes.size();
	result = new toxiclab_hub[length];
	int cnt = 0;
	foreach(HandshakePtr p, handshakes)
	{
		result[cnt].num = cnt+1;
		result[cnt].address = p->GetEndpoint().ToString();
		if(p->Connecting())
			result[cnt].status = "connecting";
		else
			result[cnt].status = "handshaking";
		++cnt;
	}

	return length;
}

void toxiclab_showDownload(ID id, toxiclab_downloadResult& result){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_showDownload");
	DownloadPtr pdl = System::GetDownloadMgr()->Get(id);
	Share::FileInfo inf = pdl->GetFileInfo().Info();

	result.id = inf.partial;
	result.name = inf.name;
	result.path = inf.path;
	result.size = inf.size;
	result.hrsize =  Ui::HumanReadable(inf.size);
	result.progress = Ui::Progress(pdl->GetFileMap()->CountBytes(Integrity::FULL | Integrity::VALID), pdl->GetFileMap()->CountBytes());
	result.downloaders = pdl->CountSources();
	result.rate = Ui::HumanReadable(pdl->AvrRate());
	result.sha1 = inf.sha1.Empty() ? "n/a" : inf.sha1.ToString();
	result.ttr = inf.ttr.Empty() ? "n/a" : inf.ttr.ToString();
	result.md5 = inf.md5.Empty() ? "n/a" : inf.md5.ToString();
	result.ed2k = inf.ed2k.Empty() ? "n/a" : inf.ed2k.ToString();
	result.bp =  (inf.sha1.Empty() || inf.ttr.Empty()) ? "n/a" : inf.sha1.ToString() + "." + inf.ttr.ToString();
	result.tiger_min =  inf.tigerTree.Empty() ? "n/a" : boost::lexical_cast<std::string>(inf.tigerTree.MinimalRange());

	std::vector<FileRangeEx> ranges;
	pdl->GetFileMap()->Dump(std::back_inserter(ranges));
	result.range_length = ranges.size();
	result.range = new toxiclab_fileRange[result.range_length];
	int cnt = 0;
	foreach(FileRangeEx r, ranges)
	{
		uint percent = 100 * double(double(r.Length()) / inf.size);
		result.range[cnt].percentage = percent;

		switch(r.Status())
		{
			case Integrity::EMPTY: result.range[cnt].status = "empty"; break;
			case Integrity::FULL: result.range[cnt].status ="full"; break;
			case Integrity::VALID: result.range[cnt].status ="valid"; break;
			case Integrity::INVALID: result.range[cnt].status ="invalid"; break;
			case Integrity::ACTIVE: result.range[cnt].status ="active"; break;
			default: result.range[cnt].status = "mixed";
		}
		++cnt;
	}

	std::vector<SourceHostEx> sources;
	pdl->CopySources(std::back_inserter(sources));
	uint nsource = 0;
	cnt = 0;
	result.host_length = sources.size();
	result.hosts = new toxiclab_sourceHost[result.host_length];
	foreach(const SourceHostEx& host, sources)
	{
		++nsource;
		const std::time_t after =
			System::Now() < host.connectAfter ?
			host.connectAfter - System::Now() : 0;
		result.hosts[cnt].source_number = nsource;
		result.hosts[cnt].source_endpoint = host.endpoint.ToString();
		result.hosts[cnt].source_connection = host.connections;
		result.hosts[cnt].source_max = host.maxConnections;
		result.hosts[cnt].source_after = after;
		result.hosts[cnt].source_nick = Url::Encode(host.nick);
		++cnt;
	}
}

int toxiclab_showAllDownload(toxiclab_download*& result, string sortBy, bool forward){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_showAllDownload");
	std::vector<DownloadPtr> dls;
	System::GetDownloadMgr()->Dump(std::back_inserter(dls));
	//sorting option
	Ui::SortOption so;
	if(sortBy == "name")
		so.SortBy(Ui::Tags::name);
	else if(sortBy == "size")
		so.SortBy(Ui::Tags::size);
	else if(sortBy == "done")
		so.SortBy(Ui::Tags::done);
	else if(sortBy == "rate")
		so.SortBy(Ui::Tags::done);
	else if(sortBy == "sources")
		so.SortBy(Ui::Tags::sources);
	else if(sortBy == "priority")
		so.SortBy(Ui::Tags::priority);
	so.Forward(forward);

	Ui::Comparers::Download comparer(so);
	//std::sort(dls.begin(), dls.end(), comparer);

	int total_download = dls.size();
	result = new toxiclab_download[total_download];
	for(uint i = 0; i < dls.size(); ++i)
	{
		DownloadPtr p = dls[i];
		Share::FileInfo inf = p->GetFileInfo().Info();

		result[i].download_name = Ui::AdaptString(inf.name);
		result[i].download_size = Ui::HumanReadable(inf.size);
		result[i].download_rate  = Ui::HumanReadable(p->AvrRate());
		result[i].sources = p->CountSources();
		result[i].active_sources =  p->CountDownloaders();
		result[i].progress = Ui::Progress(p->GetFileMap()->CountBytes(Integrity::FULL | Integrity::VALID), p->GetFileMap()->CountBytes());
		result[i].download_id =  inf.partial;
		result[i].priority = int(p->Priority());
		result[i].dl_paused = p->Status() == Download::PAUSE;
		result[i].downloaded_byte = Ui::HumanReadable(p->GetFileMap()->CountBytes(Integrity::FULL | Integrity::VALID));

		switch(p->Status())
		{
			case Download::TRANSFER: result[i].status = "transfer"; break;
			case Download::HASH: result[i].status = "hash"; break;
			case Download::MOVE: result[i].status = "move"; break;
			case Download::PAUSE: result[i].status = "paused"; break;
			case Download::STOP: result[i].status = "queued"; break;
			case Download::ERROR: result[i].status = "error"; break;
			default: assert(!"can't be here");
		}
	}
	return total_download;
}

bool toxiclab_createDownload(ID search_id, ID result_id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_createDownload");
	SearcherPtr p;
	ID id;
	try
	{
		//modify to sea
		p = System::GetSearchMgr()->Get(search_id);
		id = result_id;
		//end of modification.
	}
	catch(Search::Manager::Error&)
	{
		__android_log_write(ANDROID_LOG_DEBUG, "sharelin" , "Bad search");
		return false;
	}

	Search::Result r;
	try { r = p->GetResult(id); }
	catch (std::runtime_error&)
	{
		__android_log_write(ANDROID_LOG_DEBUG, "sharelin" , "Bad result");
		return false;
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
		__android_log_write(ANDROID_LOG_DEBUG, "sharelin" , std::string(err.what()).c_str());
		return false;
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
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from toxiclab_createDownload");
	return true;
}

int toxiclab_showIncoming(toxiclab_incoming*& result){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_showIncoming");
	std::vector<std::string> paths;

	ScanDirectory(System::GetSettings()->general.complete, std::back_inserter(paths), FileSystem::REGULAR_FILE, FileSystem::NOT_FOLLOW);

	int length = paths.size();
	result = new toxiclab_incoming[length];

	unsigned n = 0;
	foreach(const std::string& s, paths)
	{
		result[n].name = Ui::AdaptString(FileSystem::FileName(s));
		result[n].size = Ui::HumanReadable(FileSystem::FileSize(s));
		result[n].type = Ui::FileType(s);
		result[n].parent = System::GetSettings()->general.complete;
		++n;
	}

	return length;
}

void toxiclab_pauseDownload(ID id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_pauseDownload");
	System::GetDownloadMgr()->Get(id)->Pause();
	System::GetDownloadMgr()->Update();
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from toxiclab_pauseDownload");
}

void toxiclab_resumeDownload(ID id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_resumeDownload");
	System::GetDownloadMgr()->Get(id)->Resume();
	System::GetDownloadMgr()->Update();
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from toxiclab_resumeDownload");
}

void toxiclab_deleteDownload(ID id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_deleteDownload");
	System::GetDownloadMgr()->Get(id)->Cancel();
	System::GetDownloadMgr()->Update();
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from toxiclab_deleteDownload");
}

void toxiclab_pauseSearch(ID id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_pauseSearch");
	System::GetSearchMgr()->Get(id)->SetRate(0);
}

void toxiclab_deleteSearch(ID id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_deleteSearch");
	SearcherPtr p = System::GetSearchMgr()->Get(id);
	System::GetSearchMgr()->Detach(p);
}

void toxiclab_resumeSearch(ID id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "toxiclab_resumeSearch");
	System::GetSearchMgr()->Get(id)->SetRate(Search::Manager::BaseRate());
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    createSearch
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_toxiclab_droidg2_main_createSearch
  (JNIEnv * env, jobject obj, jstring dn, jstring min, jstring max, jstring media, jstring ext){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Java_org_toxiclab_droidg2_main_createSearch");

	const char* c_str_temp;
	jboolean isCopy;

	c_str_temp = env->GetStringUTFChars(dn, &isCopy);
	string dn_ = string(c_str_temp);
	if(isCopy == JNI_TRUE) env->ReleaseStringUTFChars(dn, c_str_temp);

	c_str_temp = env->GetStringUTFChars(min, &isCopy);
	string min_ = string(c_str_temp);
	if(isCopy == JNI_TRUE) env->ReleaseStringUTFChars(min, c_str_temp);

	c_str_temp = env->GetStringUTFChars(max, &isCopy);
	string max_ = string(c_str_temp);
	if(isCopy == JNI_TRUE) env->ReleaseStringUTFChars(max, c_str_temp);

	c_str_temp = env->GetStringUTFChars(media, &isCopy);
	string media_ = string(c_str_temp);
	if(isCopy == JNI_TRUE) env->ReleaseStringUTFChars(media, c_str_temp);

	c_str_temp = env->GetStringUTFChars(ext, &isCopy);
	string ext_ = string(c_str_temp);
	if(isCopy == JNI_TRUE) env->ReleaseStringUTFChars(ext, c_str_temp);

	toxiclab_createSearch(dn_, min_, max_, media_, ext_);
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    showSearch
 * Signature: ()[Lorg/toxiclab/droidg2/toxiclab_search;
 */
JNIEXPORT jobjectArray JNICALL Java_org_toxiclab_droidg2_main_showSearch
  (JNIEnv * env, jobject obj){
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Java_org_toxiclab_droidg2_main_showSearch");
	char debug[50];
	toxiclab_search * result_ptr = 0;
	int len =  toxiclab_showSearches(result_ptr);

	jclass clazz = env->FindClass("org/toxiclab/droidg2/jni/toxiclab_search");
	jobjectArray array;
	array = env->NewObjectArray(len, clazz, NULL);
	jmethodID mid = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");

	jstring criteria, hits, active, endless, extensions;
	jint id;

	for(int i=0; i<len; ++i){
		criteria = env->NewStringUTF(result_ptr[i].criteria.c_str());
		hits = env->NewStringUTF(result_ptr[i].hits.c_str());
		active = env->NewStringUTF(result_ptr[i].active.c_str());
		endless = env->NewStringUTF(result_ptr[i].endless.c_str());
		id = static_cast<jint>(result_ptr[i].id);
		extensions = env->NewStringUTF(result_ptr[i].extensions.c_str());
		jobject obj = env->NewObject(clazz, mid, criteria, hits, active, endless, extensions, id);
		env->SetObjectArrayElement(array, i, obj);
		env->DeleteLocalRef(obj);
		env->DeleteLocalRef(criteria);
		env->DeleteLocalRef(hits);
		env->DeleteLocalRef(active);
		env->DeleteLocalRef(endless);
		env->DeleteLocalRef(extensions);
	}

	delete[] result_ptr;
	return array;
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    showSearchResult
 * Signature: (ILjava/lang/String;Z)[Lorg/toxiclab/droidg2/toxiclab_searchResult;
 */
JNIEXPORT jobjectArray JNICALL Java_org_toxiclab_droidg2_main_showSearchResult
  (JNIEnv * env, jobject obj, jint id, jstring sortBy, jboolean forward){
//	char debug[50];
//	sprintf(debug, "Java_org_toxiclab_droidg2_main_showSearchResult %d", id);
//	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	toxiclab_searchResult * result_ptr = 0;
	string _sortBy;
	bool _forward = forward == JNI_TRUE ? true : false;
	jboolean isCopy;

	const char* temp;
	temp = env->GetStringUTFChars(sortBy, &isCopy);
	_sortBy = string(temp);
	if(isCopy == JNI_TRUE) env->ReleaseStringUTFChars(sortBy, temp);

	int len = toxiclab_showSearchResult(id, _sortBy, _forward, result_ptr);

	jclass clazz = env->FindClass("org/toxiclab/droidg2/jni/toxiclab_searchResult");
	jobjectArray array;
	array = env->NewObjectArray(len, clazz, NULL);
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "before method id");
	jmethodID mid = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;IILjava/lang/String;Ljava/lang/String;I)V");
	//sprintf(debug, "after method id...%d", mid);
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);

	jstring name, size, type;
	jint rid, sid;
	jint sources;

	//create the object array
	for(int i=0; i<len; ++i){
		name = env->NewStringUTF(result_ptr[i].name.c_str());
		rid = static_cast<jint>(result_ptr[i].rid);
		sid = static_cast<jint>(result_ptr[i].sid);
		size = env->NewStringUTF(result_ptr[i].size.c_str());
		type = env->NewStringUTF(result_ptr[i].type.c_str());
		sources = result_ptr[i].sources;
		jobject obj = env->NewObject(clazz, mid, name, rid, sid, size, type, sources);
		env->SetObjectArrayElement(array, i, obj);
		env->DeleteLocalRef(name);
		env->DeleteLocalRef(size);
		env->DeleteLocalRef(type);
		env->DeleteLocalRef(obj);
	}

	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return fom Java_org_toxiclab_droidg2_main_showSearchResult");
	delete[] result_ptr;
	return array;
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    showHubs
 * Signature: ()[Lorg/toxiclab/droidg2/toxiclab_hub;
 */
JNIEXPORT jobjectArray JNICALL Java_org_toxiclab_droidg2_main_showHubs
  (JNIEnv * env, jobject obj){
	char debug[50];
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Java_org_toxiclab_droidg2_main_showHubs");
	toxiclab_hub * result_ptr = 0;
	int length = toxiclab_showHubs(result_ptr);
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from toxiclab_showHub");
	jclass clazz = env->FindClass("org/toxiclab/droidg2/jni/toxiclab_hub");
	jobjectArray array;
	array = env->NewObjectArray(length, clazz, NULL);
	jmethodID mid = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

	jstring num, address, status;

	for(int i=0; i<length; ++i){
		num = env->NewStringUTF(result_ptr[i].num.c_str());
		address = env->NewStringUTF(result_ptr[i].address.c_str());
		status = env->NewStringUTF(result_ptr[i].status.c_str());
		jobject obj = env->NewObject(clazz, mid, num, address, status);
		env->SetObjectArrayElement(array, i, obj);
		env->DeleteLocalRef(num);
		env->DeleteLocalRef(address);
		env->DeleteLocalRef(status);
	}

	delete[] result_ptr;
	//sprintf(debug, "There are %d hubs returned", length);
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	return array;
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    showHandshake
 * Signature: ()[Lorg/toxiclab/droidg2/toxiclab_hub;
 */
JNIEXPORT jobjectArray JNICALL Java_org_toxiclab_droidg2_main_showHandshake
  (JNIEnv * env, jobject obj){
	char debug[50];
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Java_org_toxiclab_droidg2_main_showHandshake");
	toxiclab_hub * result_ptr = 0;
	int length = toxiclab_showHandshake(result_ptr);
	jclass clazz = env->FindClass("org/toxiclab/droidg2/jni/toxiclab_hub");
	jobjectArray array;
	array = env->NewObjectArray(length, clazz, NULL);
	jmethodID mid = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

	jstring num, address, status;

	for(int i=0; i<length; ++i){
		num = env->NewStringUTF(result_ptr[i].num.c_str());
		address = env->NewStringUTF(result_ptr[i].address.c_str());
		status = env->NewStringUTF(result_ptr[i].status.c_str());
		jobject obj = env->NewObject(clazz, mid, num, address, status);
		env->SetObjectArrayElement(array, i, obj);
		env->DeleteLocalRef(num);
		env->DeleteLocalRef(address);
		env->DeleteLocalRef(status);
	}

	delete[] result_ptr;
	//sprintf(debug, "There are %d handshakes returned", length);
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	return array;
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    showDownload
 * Signature: (I)Lorg/toxiclab/droidg2/toxiclab_downloadResult;
 */
JNIEXPORT jobject JNICALL Java_org_toxiclab_droidg2_main_showDownload
  (JNIEnv * env, jobject obj, jint id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Java_org_toxiclab_droidg2_main_showDownload");
	uint ID = id;
	toxiclab_downloadResult result;
	toxiclab_showDownload(ID, result);

	//create the object
	jstring name, path, size, hrsize, progress, downloaders, rate, sha1, ttr, md5, ed2k, bp, tiger_min;
	jobjectArray fileRange_array, host_array;

	//create the filerange array
	int fr_length = result.range_length;
	jclass clazz_ = env->FindClass("org/toxiclab/droidg2/jni/toxiclab_fileRange");
	fileRange_array = env->NewObjectArray(fr_length, clazz_, NULL);
	jmethodID mid = env->GetMethodID(clazz_, "<init>", "(ILjava/lang/String;)V");
	jstring status_;
	jint percent_;
	for(int i=0; i<fr_length; ++i){
		status_ = env->NewStringUTF(result.range[i].status.c_str());
		percent_ = result.range[i].percentage;
		jobject obj = env->NewObject(clazz_, mid, status_, percent_);
		env->SetObjectArrayElement(fileRange_array, i, obj);
		env->DeleteLocalRef(status_);
	}


	//create the host array
	int ht_length = result.host_length;
	clazz_ = env->FindClass("org/toxiclab/droidg2/jni/toxiclab_sourceHost");
	host_array = env->NewObjectArray(ht_length, clazz_, NULL);
	mid = env->GetMethodID(clazz_, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	jstring source_number_, source_endpoint_, source_connection_, source_max_, source_after_, source_nick_;
	for(int i=0; i<ht_length; ++i){
		source_number_ = env->NewStringUTF(result.hosts[i].source_number.c_str());
		source_endpoint_ = env->NewStringUTF(result.hosts[i].source_endpoint.c_str());
		source_connection_ = env->NewStringUTF(result.hosts[i].source_connection.c_str());
		source_max_ = env->NewStringUTF(result.hosts[i].source_max.c_str());
		source_after_ = env->NewStringUTF(result.hosts[i].source_after.c_str());
		source_nick_ = env->NewStringUTF(result.hosts[i].source_nick.c_str());
		jobject obj = env->NewObject(clazz_, mid, source_number_, source_endpoint_, source_connection_,
				source_max_, source_after_, source_nick_);
		env->SetObjectArrayElement(host_array, i, obj);
		env->DeleteLocalRef(source_number_);
		env->DeleteLocalRef(source_endpoint_);
		env->DeleteLocalRef(source_connection_);
		env->DeleteLocalRef(source_connection_);
		env->DeleteLocalRef(source_after_);
		env->DeleteLocalRef(source_nick_);
	}

	id = result.id;
	name = env->NewStringUTF(result.name.c_str());
	path = env->NewStringUTF(result.path.c_str());
	size = env->NewStringUTF(result.size.c_str());
	hrsize = env->NewStringUTF(result.hrsize.c_str());
	progress = env->NewStringUTF(result.progress.c_str());
	downloaders = env->NewStringUTF(result.downloaders.c_str());
	rate = env->NewStringUTF(result.rate.c_str());
	sha1 = env->NewStringUTF(result.sha1.c_str());
	ttr = env->NewStringUTF(result.ttr.c_str());
	md5 = env->NewStringUTF(result.md5.c_str());
	ed2k = env->NewStringUTF(result.ed2k.c_str());
	bp = env->NewStringUTF(result.bp.c_str());
	tiger_min = env->NewStringUTF(result.tiger_min.c_str());

	jclass clazz = env->FindClass("org/toxiclab/droidg2/jni/toxiclab_downloadResult");
	mid = env->GetMethodID(clazz, "<init>", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Lorg/toxiclab/droidg2/jni/toxiclab_fileRange;[Lorg/toxiclab/droidg2/jni/toxiclab_sourceHost)V");
	jobject obj_ = env->NewObject(clazz, mid, id, name, path, size, hrsize, progress, downloaders, rate, sha1, ttr,
			md5, ed2k, bp, tiger_min, fileRange_array, host_array);

	env->DeleteLocalRef(name);
	env->DeleteLocalRef(path);
	env->DeleteLocalRef(size);
	env->DeleteLocalRef(hrsize);
	env->DeleteLocalRef(progress);
	env->DeleteLocalRef(downloaders);
	env->DeleteLocalRef(rate);
	env->DeleteLocalRef(sha1);
	env->DeleteLocalRef(ttr);
	env->DeleteLocalRef(md5);
	env->DeleteLocalRef(ed2k);
	env->DeleteLocalRef(bp);
	env->DeleteLocalRef(tiger_min);

	return obj_;
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    showAllDownload
 * Signature: (Ljava/lang/String;Z)[Lorg/toxiclab/droidg2/toxiclab_download;
 */
JNIEXPORT jobjectArray JNICALL Java_org_toxiclab_droidg2_main_showAllDownload
  (JNIEnv * env, jobject obj, jstring sortBy, jboolean forward){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Java_org_toxiclab_droidg2_main_showAllDownload");
	string _sortBy;
	bool _forward;
	toxiclab_download * result_ptr;
	int length = toxiclab_showAllDownload(result_ptr, _sortBy, _forward);
	jclass clazz = env->FindClass("org/toxiclab/droidg2/jni/toxiclab_download");
	jobjectArray array;
	array = env->NewObjectArray(length, clazz, NULL);
	jmethodID mid = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IILjava/lang/String;IIZLjava/lang/String;Ljava/lang/String;)V");

	jstring download_name, download_size, download_rate, progress, status;
	jstring downloaded_bytes;
	jint sources, active_sources, download_id, priority;
	jboolean dl_paused;

	//create the object array
	for(int i=0; i<length; ++i){
		download_name = env->NewStringUTF(result_ptr[i].download_name.c_str());
		download_size = env->NewStringUTF(result_ptr[i].download_size.c_str());
		download_rate = env->NewStringUTF(result_ptr[i].download_rate.c_str());
		sources = static_cast<jint>(result_ptr[i].sources);
		active_sources = static_cast<jint>(result_ptr[i].active_sources);
		progress = env->NewStringUTF(result_ptr[i].progress.c_str());
		download_id = static_cast<jint>(result_ptr[i].download_id);
		priority = static_cast<jint>(result_ptr[i].priority);
		dl_paused = result_ptr[i].dl_paused ? JNI_TRUE : JNI_FALSE;
		status = env->NewStringUTF(result_ptr[i].status.c_str());
		downloaded_bytes = env->NewStringUTF(result_ptr[i].downloaded_byte.c_str());

		jobject obj_ = env->NewObject(clazz, mid, download_name, download_size, download_rate,
				sources, active_sources, progress, download_id, priority, dl_paused, status, downloaded_bytes);
		env->SetObjectArrayElement(array, i, obj_);

		env->DeleteLocalRef(download_name);
		env->DeleteLocalRef(download_size);
		env->DeleteLocalRef(download_rate);
		env->DeleteLocalRef(progress);
		env->DeleteLocalRef(status);
		env->DeleteLocalRef(downloaded_bytes);
		env->DeleteLocalRef(obj_);
	}
	//return the array
	delete[] result_ptr;
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from Java_org_toxiclab_droidg2_main_showAllDownload");
	return array;
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    createDownload
 * Signature: (II)V
 */
JNIEXPORT jboolean JNICALL Java_org_toxiclab_droidg2_main_createDownload
  (JNIEnv * env, jobject obj, jint searchID, jint resultID){
	char debug[50];
	sprintf(debug, "Java_org_toxiclab_droidg2_main_createDownload %d %d", searchID, resultID);
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	ID _searchID = searchID;
	ID _resultID = resultID;

	bool result = toxiclab_createDownload(_searchID, _resultID);
	jboolean result_;

	if(result)
		result_ = JNI_TRUE;
	else
		result_ = JNI_FALSE;
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from Java_org_toxiclab_droidg2_main_createDownload");
	return result_;
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    showIncoming
 * Signature: ()[Lorg/toxiclab/droidg2/jni/toxiclab_incoming;
 */
JNIEXPORT jobjectArray JNICALL Java_org_toxiclab_droidg2_main_showIncoming
  (JNIEnv * env , jobject obj){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Java_org_toxiclab_droidg2_main_showIncoming");
	toxiclab_incoming * result_ptr;
	int length = toxiclab_showIncoming(result_ptr);
	jclass clazz = env->FindClass("org/toxiclab/droidg2/jni/toxiclab_incoming");
	jobjectArray array;
	array = env->NewObjectArray(length, clazz, NULL);
	jmethodID mid = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	jstring name, size, type, parent;

	for(int i=0; i<length; ++i){
		name = env->NewStringUTF(result_ptr[i].name.c_str());
		size = env->NewStringUTF(result_ptr[i].size.c_str());
		type = env->NewStringUTF(result_ptr[i].type.c_str());
		parent = env->NewStringUTF(result_ptr[i].parent.c_str());

		jobject obj = env->NewObject(clazz, mid, name, size, type, parent);
		env->SetObjectArrayElement(array, i, obj);

		env->DeleteLocalRef(name);
		env->DeleteLocalRef(size);
		env->DeleteLocalRef(type);
		env->DeleteLocalRef(parent);
		env->DeleteLocalRef(obj);
	}

	delete[] result_ptr;
	return array;
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    pauseDownload
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_toxiclab_droidg2_main_pauseDownload
  (JNIEnv * env, jobject obj, jint id){
	char debug[50];
	sprintf(debug, "Java_org_toxiclab_droidg2_main_pauseDownload %d", id);
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	ID _id = static_cast<ID>(id);
	toxiclab_pauseDownload(_id);
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from Java_org_toxiclab_droidg2_main_pauseDownload");
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    resumeDownload
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_toxiclab_droidg2_main_resumeDownload
  (JNIEnv * env, jobject obj, jint id){
	char debug[50];
	sprintf(debug, "Java_org_toxiclab_droidg2_main_resumeDownload %d", id);
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	ID _id = static_cast<ID>(id);
	toxiclab_resumeDownload(_id);
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from Java_org_toxiclab_droidg2_main_resumeDownload");
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    deleteDownload
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_toxiclab_droidg2_main_deleteDownload
  (JNIEnv * env, jobject obj, jint id){
	char debug[50];
	sprintf(debug, "Java_org_toxiclab_droidg2_main_deleteDownload %d", id);
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	ID _id = static_cast<ID>(id);
	toxiclab_deleteDownload(_id);
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from Java_org_toxiclab_droidg2_main_deleteDownload");
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    pauseSearch
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_toxiclab_droidg2_main_pauseSearch
  (JNIEnv * env, jobject obj, jint id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Java_org_toxiclab_droidg2_main_pauseSearch");
	ID _id = static_cast<ID>(id);
	toxiclab_pauseSearch(_id);
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    deleteSearch
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_toxiclab_droidg2_main_deleteSearch
  (JNIEnv * env, jobject obj, jint id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Java_org_toxiclab_droidg2_main_deleteSearch");
	ID _id = static_cast<ID>(id);
	toxiclab_deleteSearch(_id);
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    resumeSearch
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_toxiclab_droidg2_main_resumeSearch
  (JNIEnv * env, jobject obj, jint id){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Java_org_toxiclab_droidg2_main_resumeSearch");
	ID _id = static_cast<ID>(id);
	toxiclab_resumeSearch(_id);
}

/*
 * Class:     org_toxiclab_droidg2_main
 * Method:    killCore
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_toxiclab_droidg2_main_killCore
  (JNIEnv * env, jobject obj){
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "enter Java_org_toxiclab_droidg2_main_killCore");
	System::Shutdown();
	__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "return from Java_org_toxiclab_droidg2_main_killCore");
}




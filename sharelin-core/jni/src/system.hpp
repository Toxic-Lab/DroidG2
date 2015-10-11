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

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "ioforward.hpp"
#include <ctime>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include "ip.hpp"
#include "guid.hpp"
#include "logger.hpp"

class G2Net;
class DownloadManager;
namespace Search {class Manager;}
class WebUIServer;
class FileManager;
class Settings;
class HubCache;
class UploadManager;
namespace async_task { class Server; }
namespace Share { class Manager; }
namespace History { class Keeper; }
class TrafLimit;
class TrafManager;
class Discovery;
class UpdatesWatch;
class Security;

class SystemImpl
{
public:
	SystemImpl(ba::io_service& ioref);
	int Run();
	void Shutdown();
	void OnTimer(std::time_t);
	void VerboseLevel(unsigned int);
	void UpdateSettings();
	~SystemImpl();

	ba::io_service& io;
	boost::scoped_ptr <Settings> pSettings;
	boost::scoped_ptr <G2Net> pNet;
	boost::scoped_ptr <DownloadManager> pDownloadManager;
	boost::scoped_ptr <UploadManager> pUploadManager;
	boost::scoped_ptr <Search::Manager> pSearchMgr;
	boost::scoped_ptr <WebUIServer> pWebUI;
	boost::scoped_ptr <FileManager> pFileManager;
	boost::scoped_ptr <Share::Manager> pShareManager;
	boost::scoped_ptr <HubCache> pHubCache;
	boost::scoped_ptr <async_task::Server> pTaskServer;
	boost::scoped_ptr <TrafManager> pDownloadTraf;
	boost::scoped_ptr <TrafManager> pUploadTraf;
	boost::shared_ptr <Discovery> pDiscovery;
	boost::scoped_ptr <History::Keeper> pHistory;
	boost::scoped_ptr <Security> pSecurity;
	boost::shared_ptr <UpdatesWatch> pUpdates;

	Logger logger;


	std::time_t now;
	GUID guid;
	std::string home;
	std::string logFile;
	std::string confFile;

private:
	void Init();
	void Stop();
	void LoadSettings();
	void LoadHubs();
};

namespace System
{
	ba::io_service& GetIo();
	Settings* GetSettings();
	G2Net* GetNetwork();
	DownloadManager* GetDownloadMgr();
	UploadManager* GetUploadMgr();
	Search::Manager* GetSearchMgr();
	Share::Manager* GetShareMgr();
	HubCache* GetHubCache();
	async_task::Server* GetTaskServer();
	TrafManager* GetTraffIn();
	TrafManager* GetTraffOut();
	FileManager* GetFileMgr();
	WebUIServer* GetWebUi();
	History::Keeper* GetHistory();
	UpdatesWatch* GetUpdatesWatch();
	Security* GetSecurity();

	void Shutdown();
	void UpdateSettings();
	std::time_t Now();
	std::time_t Since(std::time_t);
	GUID Guid();
	Ip::Endpoint SelfEndpoint();
	bool IsFirewalled();
	Logger& Log();
	Logger& LogBas();
	Logger& LogAdv();
	Logger& LogDev();
	std::string Home();
	std::string LogFileName();
}

#endif //SYSTEM_HPP


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

#include <string>
#include <stdexcept>

#include <boost/bind.hpp>

#include "system.hpp"
#include "g2net.hpp"
#include "downloadmanager.hpp"
#include "searchmanager.hpp"
#include "webuiserver.hpp"
#include "filemanager.hpp"
#include "sharemanager.hpp"
#include "settings.hpp"
#include "identity.hpp"
#include "env.hpp"
#include "io.hpp"
#include "filesystem.hpp"
#include "hubcache.hpp"
#include "asynctaskserver.hpp"
#include "uploadmanager.hpp"
#include "discovery.hpp"
#include "history.hpp"
#include "trafmanager.hpp"
#include "updateswatch.hpp"
#include "security.hpp"
#include <android/log.h>

namespace {
	SystemImpl* pSystem = 0;

	const std::time_t HANDLE_LOG_PERIOD = 60;
}

SystemImpl::SystemImpl(ba::io_service& ioref):
			io(ioref),
			pSettings(new Settings),
			pNet(new G2Net(io)),
			pDownloadManager(new DownloadManager),
			pUploadManager(new UploadManager),
			pSearchMgr(new Search::Manager),
			pWebUI(new WebUIServer(io)),
			pFileManager(new FileManager),
			pShareManager(new Share::Manager()),
			pHubCache(new HubCache(10000)),
			pTaskServer(new async_task::Server(io, 5)),
			pDownloadTraf(new TrafManager),
			pUploadTraf(new TrafManager),
			pDiscovery(new Discovery(io)),
			pHistory(new History::Keeper),
			pSecurity(new Security),
			pUpdates(new UpdatesWatch(io))
{
	pSystem = this;
}

SystemImpl::~SystemImpl()
{
}

void SystemImpl::VerboseLevel(unsigned int n)
{
	logger.VerboseLevel(n);
}

void SystemImpl::UpdateSettings()
{
	pSettings->Save(confFile);
	logger.MaxSize(pSettings->general.logSize * 1024 * 1024);
	pDownloadTraf->SetShare(pSettings->net.maxBwIn * 0.95);
	pUploadTraf->SetShare(pSettings->net.maxBwOut * 0.95);
	pWebUI->UpdateSettings();
}

void SystemImpl::LoadSettings()
{
	System::LogBas() << "Loading settings" << std::endl;
	pSettings->Init(home);
	if(FileSystem::Exists(confFile))
	{
		try{ pSettings->Load(confFile); }
		catch(std::exception& e)
		{
			System::LogBas() << "Couldn't load settings: " << e.what() << std::endl;
		}
	}
	else 
		System::LogBas() << "Config (" << confFile << ") not found. Default settings loaded" << std::endl;
}

void SystemImpl::LoadHubs()
{
	const std::string hubsFile = home + "/hubs.dat";

	if(!FileSystem::Exists(hubsFile)) return;

	System::LogBas() << "Loading hubs list" << std::endl;
	try 
	{ 
		pHubCache->Load(home + "/hubs.dat"); 
		System::LogBas() << "Hub cache size: " << pHubCache->Size() << std::endl;
	}
	catch (std::exception& e)
	{
		System::LogBas() << "Couldn't load hubs list: " << e.what() << std::endl;
	}
}

void SystemImpl::Init()
{
	home = Env::HomeFolder() + "/.sharelin";
	guid.Generate();
	logFile = home + "/sharelin.log";
	confFile = home + "/sharelin.conf";

	FileSystem::PrepareFolder(home);

	logger.Open(logFile, 1024 * 1024);

	System::LogBas() << "Starting " << Ident::name << std::endl;

	LoadSettings();
	logger.MaxSize(pSettings->general.logSize * 1024 * 1024);

	FileSystem::PrepareFolder(pSettings->general.complete);
	FileSystem::PrepareFolder(pSettings->general.incomplete);
	FileSystem::PrepareFolder(pSettings->general.incomplete + "/inf");

	pDownloadTraf->SetShare(pSettings->net.maxBwIn * 0.95);
	pUploadTraf->SetShare(pSettings->net.maxBwOut * 0.95);

	pSecurity->Start();
	LoadHubs();
	pShareManager->Start();
	pNet->Start();
	pDownloadManager->Start();
	pWebUI->Start();
	pSearchMgr->Start();
	pDiscovery->Start();
	pUpdates->Start();
}

int SystemImpl::Run()
{
	try { Init(); }
	catch(std::exception& e)
	{
		System::LogBas() << "Couldn't start because of error: " << e.what() << std::endl;
		return 1;
	}
	
	System::LogBas() << "Running io" << std::endl;

	io.run();

	return 0;
}

void SystemImpl::OnTimer(std::time_t n)
{
	now = n;
	pSearchMgr->OnTimer();
	pNet->OnTimer();
	pDownloadManager->OnTimer();
	pDownloadTraf->OnTimer(now);
	pUploadTraf->OnTimer(now);
	pShareManager->OnTimer( now );
	pUploadManager->OnTimer( now );
}

void SystemImpl::Shutdown()
{
	io.post(boost::bind(&SystemImpl::Stop, this));
}

void SystemImpl::Stop()
{
	System::LogBas() << "Stopping io" << std::endl;
	io.stop();

	System::LogBas() << "Saving settings" << std::endl;
	pSettings->Save(confFile);

	System::LogBas() << "Aborting async tasks" << std::endl;
	pTaskServer->AbortAll();

	System::LogBas() << "Shutting down G2 network" << std::endl;
	pNet->Shutdown();

	System::LogBas() << "Shutting down download manager" << std::endl;
	pDownloadManager->Shutdown();

	System::LogBas() << "Shutting down upload manager" << std::endl;
	pUploadManager->Shutdown();

	System::LogBas() << "Shutting down share manager" << std::endl;
	pShareManager->Shutdown();

	pUpdates->Shutdown();
	pDiscovery->Shutdown();

	System::LogBas() << "Saving hubs list" << std::endl;
	try { pHubCache->Save(home + "/hubs.dat"); }
	catch(std::exception& e)
	{
		System::LogBas() << "Couldn't save hubs list: " << e.what() << std::endl;
	}

	System::LogBas() << "Bye!" << std::endl;
}

namespace System
{
	ba::io_service& GetIo() { return pSystem->io; }
	Settings* GetSettings() { return pSystem->pSettings.get(); }
	G2Net* GetNetwork() { return pSystem->pNet.get(); }
	DownloadManager* GetDownloadMgr() { return pSystem->pDownloadManager.get(); }
	HubCache* GetHubCache() { return pSystem->pHubCache.get(); }
	async_task::Server* GetTaskServer() { return pSystem->pTaskServer.get(); }
	TrafManager* GetTraffIn() { return pSystem->pDownloadTraf.get(); }
	TrafManager* GetTraffOut() { return pSystem->pUploadTraf.get(); }
	Share::Manager* GetShareMgr() { return pSystem->pShareManager.get(); }
	FileManager* GetFileMgr() { return pSystem->pFileManager.get(); }
	Search::Manager* GetSearchMgr() { return pSystem->pSearchMgr.get(); }
	UploadManager* GetUploadMgr() { return pSystem->pUploadManager.get(); }
	WebUIServer* GetWebUi() { return pSystem->pWebUI.get(); }
	History::Keeper* GetHistory() { return pSystem->pHistory.get(); }
	UpdatesWatch* GetUpdatesWatch() { return pSystem->pUpdates.get(); }
	Security* GetSecurity() { return pSystem->pSecurity.get(); }

	Logger& Log() { return pSystem->logger; }
	Logger& LogBas() { return pSystem->logger.Log(1); }
	Logger& LogAdv() { return pSystem->logger.Log(2); }
	Logger& LogDev() { return pSystem->logger.Log(3); }

	std::string Home() { return pSystem->home; }
	void Shutdown() { return pSystem->Shutdown(); }
	std::string LogFileName() { return pSystem->logFile; }
	void UpdateSettings() { pSystem->UpdateSettings(); }

	GUID Guid() { return pSystem->guid; }
	std::time_t Now() { return pSystem->now; }
	std::time_t Since(std::time_t t)
	{
		const std::time_t now = Now();
		if(now < t) throw std::logic_error("Timestamp from the future!");
		return now - t;
	}

	Ip::Endpoint SelfEndpoint() { return GetNetwork()->SelfEndpoint(); }
	bool IsFirewalled() { return GetNetwork()->IsFirewalled(); }

}


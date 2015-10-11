/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008  Andrey Stroganov

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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP
#include "types.hpp"
#include <map>
#include <vector>
#include <string>
#include "ip.hpp"
#include "ipsegment.hpp"

namespace Option
{
	const char userName[] = "UserName";
	const char maxHubs[] = "MaxHubs";
	const char statisticsFile[] = 		"Statistics";
	const char maxBwIn[] = 			"MaxBWin";
	const char maxBwOut[] = 		"MaxBWout";
	const char listenPort[] = 		"ListenPort";
	const char listenIp[] = 		"ListenIP";
	const char maxUploads[] = 		"MaxUploads";
	const char maxDownloads[] = 		"MaxDownloads";
	const char maxFiles[] = 		"MaxConcurrentFiles";
	const char directAble[] = 		"DirectAble";
	const char share[] = 			"Share";
	const char telnetPort[] = 		"TelnetPort";
	const char queueSize[] =	 	"QueueSize";
	const char uploadsPerHost[] = 		"UploadsPerHost";
	const char webuiPort[] = 		"WebUIPort";
	const char webuiAllowIp[] = 		"WebUIAllowIP";
	const char remotePort[] = 		"RemotePort";
	const char remoteRanges[] = 		"RemoteRanges";
	const char complete[] = 		"Complete";
	const char incomplete[] = 		"Incomplete";
	const char hashRate[] = 		"HashRate";
	const char webuiStyle[] = 		"WebUIStyle";
	const char webuiPageLines[] = 		"wuiPageLines";
	const char logSize[] = 			"logSize";
}

struct Settings
{
	typedef std::vector<std::string> SharedFolders;

	enum {YES, NO, AUTO};

	struct 
	{
		std::string userName;
		std::string statisticsFile;
		std::string incomplete;
		std::string complete;
		std::vector<IpSegment> uisegments;
		uint logSize; //mb
	} general;
	
	struct  
	{
		uint maxHubs;
		uint maxBwIn;
		uint maxBwOut;
		uint16 listenPort;
		Ip::Address listenIp;
		uint maxUploads;
		uint maxDownloads;
		uint maxFiles;
		uint queueSize;
		uint pollMin;
		uint pollMax;
		uint uploadsPerHost;
		uint directAble; //0-auto, 1-yes, 2-no
		uint qkeyPeriod;
	} net;
	
	struct 
	{
		SharedFolders folders;
		uint hashRate;
	} share;
	
	struct 
	{
		uint16 port;
	} telnet;

	struct 
	{
		std::string style;
		uint16 port;
		uint pageLines;
	} webui;

	void Load(const std::string&);
	void Save(const std::string&);
	void Init(const std::string&);
	void Set(const std::string& name, const std::string& value);
};

#endif //SETTINGS_HPP


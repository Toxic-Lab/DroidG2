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

#include <fstream>
#include "settings.hpp"
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "env.hpp"
#include <iostream>
#include "system.hpp"
#include "foreach.hpp"
#include <android/log.h>

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::trim;

void Settings::Init(const std::string& home)
{
	general.userName = 		"User";
	general.statisticsFile = 		"";
	general.incomplete = 	home + "/incomplete";
	general.complete = 	home + "/complete";
	general.logSize = 	5;

	net.queueSize =		10;
	net.maxHubs = 		2;
	net.maxBwIn = 		32 * 1024;
	net.maxBwOut = 		32 * 1024;
	net.listenPort = 		6349;
	net.listenIp = 		0;
	net.maxUploads = 		6;
	net.maxDownloads = 	50;
	net.maxFiles = 50;
	net.pollMin =			40;
	net.pollMax = 		120;
	net.uploadsPerHost =	2;
	net.directAble	=	AUTO;
	net.qkeyPeriod = 		60 * 60;

	webui.port =		0; //off if 0
	webui.style = "default";
	webui.pageLines = 50;

	telnet.port =		0; //off if 0

	share.hashRate = 		0; //MBps
}

void Settings::Save(const std::string& fname)
{
	std::ofstream f(fname.c_str());
	if(!f.good()) throw std::runtime_error("File open error");

	f << Option::userName << " = " << general.userName << std::endl;
	f << Option::logSize << " = " << general.logSize << std::endl;
	f << Option::maxHubs << " = " << net.maxHubs << std::endl;
	f << Option::maxBwIn << " = " << net.maxBwIn / 1024 << std::endl;
	f << Option::maxBwOut << " = " << net.maxBwOut / 1024 << std::endl;
	f << Option::listenPort << " = " << net.listenPort << std::endl;
	f << Option::maxUploads << " = " << net.maxUploads << std::endl;
	f << Option::maxDownloads << " = " << net.maxDownloads << std::endl;
	f << Option::maxFiles << " = " << net.maxFiles << std::endl;

	f << Option::directAble << " = ";
       	switch(net.directAble)
	{
		case YES: f << "yes"; break;
		case NO: f << "no"; break;
		case AUTO: f << "auto"; break;
	}
	f << std::endl;

	f << Option::hashRate << " = " << share.hashRate << std::endl;
	f << Option::complete << " = " << general.complete << std::endl;
	f << Option::incomplete << " = " << general.incomplete << std::endl;
	f << Option::uploadsPerHost << " = " << net.uploadsPerHost << std::endl;
	f << Option::queueSize << " = " << net.queueSize << std::endl;
	f << Option::webuiPort << " = " << webui.port << std::endl;
	f << Option::webuiStyle << " = " << webui.style << std::endl;
	f << Option::webuiPageLines << " = " << webui.pageLines << std::endl;

	f << Option::share << " = ";
	for(SharedFolders::iterator i = share.folders.begin(); i != share.folders.end(); ++i) 
		f << *i << ";";
	f << std::endl;
	
	f << Option::remoteRanges << " = ";
	foreach(IpSegment s, general.uisegments)
	{
		if(s.First() == s.Last())
			f << s.First();
		else
			f << s.First() << "-" << s.Last();
		f << "; ";
	}
	f << std::endl;
	
	f << Option::telnetPort << " = " << telnet.port << std::endl;
	
}

void Settings::Set(const std::string& name, const std::string& value)
{
	using namespace boost;

	if(iequals(name, Option::userName)) general.userName = value;
	else if(iequals(name, Option::maxHubs)) net.maxHubs = lexical_cast<uint>(value);
	else if(iequals(name, Option::maxBwIn)) net.maxBwIn = 1024 * lexical_cast<uint>(value);
	else if(iequals(name, Option::maxBwOut)) net.maxBwOut = 1024 * lexical_cast<uint>(value);
	else if(iequals(name, Option::listenPort)) net.listenPort = lexical_cast<uint>(value);
	else if(iequals(name, Option::queueSize)) net.queueSize = lexical_cast<uint>(value);
	else if(iequals(name, Option::maxUploads)) net.maxUploads = lexical_cast<uint>(value);
	else if(iequals(name, Option::maxDownloads)) net.maxDownloads = lexical_cast<uint>(value);
	else if(iequals(name, Option::maxFiles)) net.maxFiles = lexical_cast<uint>(value);
	else if(iequals(name, Option::directAble))
	{
		if(boost::iequals(value, "yes")) net.directAble = YES;
		else if(boost::iequals(value, "no")) net.directAble = NO;
		else net.directAble = AUTO;
	}
	else if(iequals(name, Option::uploadsPerHost)) net.uploadsPerHost = lexical_cast<uint>(value);
	else if(iequals(name, Option::logSize)) general.logSize = lexical_cast<uint>(value);
	else if(iequals(name, Option::complete)) 
	{
		general.complete = value;
		if(general.complete.empty()) general.complete = System::Home() + "/complete";
		if(general.complete[0] == '~')
			general.complete.replace(0, 1, Env::HomeFolder());
	}
	else if(iequals(name, Option::incomplete)) 
	{
		general.incomplete = value;
		if(general.incomplete.empty()) general.incomplete = System::Home() + "/incomplete";
		if(general.incomplete[0] == '~')
			general.incomplete.replace(0, 1, Env::HomeFolder());
	}
	else if(iequals(name, Option::telnetPort)) telnet.port = lexical_cast<uint>(value);
	else if(iequals(name, Option::webuiPort)) webui.port = lexical_cast<uint>(value);
	else if(iequals(name, Option::hashRate)) share.hashRate = lexical_cast<uint>(value);
	else if(iequals(name, Option::webuiStyle)) webui.style = value.empty() ? "default" : value;
	else if(iequals(name, Option::webuiPageLines)) webui.pageLines = lexical_cast<uint>(value);
	else if(iequals(name, Option::share))
	{
		/*
		std::stringstream ss(value);
		std::string path;
		while(std::getline(ss, path, ';'))
		{
		*/

		share.folders.clear();
		std::vector<std::string> folders;
		split(folders, value, is_any_of(";"));
		foreach(std::string f, folders)
		{
			trim(f);
			if(f.empty()) continue;
			if(f[0] == '~') f.replace(0, 1, Env::HomeFolder());
			trim_right_if(f, is_any_of("/"));
			if(!f.empty()) share.folders.push_back(f);
		}

		/*for(SharedFolders::iterator i = share.folders.begin(); i != share.folders.end(); ++i)
		{
			trim(*i);
			trim_right_if(*i, is_any_of("/"));
		}
		*/
	}
	else if(iequals(name, Option::remoteRanges))
	{
		std::stringstream s(value);
		std::string strRange;
		general.uisegments.clear();
		while(std::getline(s, strRange, ';'))
		{
			trim(strRange);
			if(strRange.empty()) continue;

			Ip::Address ip1, ip2;
			const std::size_t pos = strRange.find('-');

			if(pos == std::string::npos)
			{
				if(!ip1.FromString(strRange)) 
					throw std::runtime_error("Bad ip range: " + strRange);
				ip2 = ip1;
			}
			else
			{
				if(!ip1.FromString(strRange.substr(0, pos)) || !ip2.FromString(strRange.substr(pos + 1)))
					throw std::runtime_error("Bad ip range: " + strRange);
			}

			general.uisegments.push_back(IpSegment(ip1, ip2));
		}
	}
	else throw std::runtime_error("Unknown option: " + name); 
}

void Settings::Load(const std::string& fname)
{
	std::ifstream f(fname.c_str());

	if(!f.good()) {
		__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "File open error");
		throw std::runtime_error("File open error");
	}

	std::string line;
	int lineNumber = 0;
	while(std::getline(f, line))
	{
		++lineNumber;

		std::stringstream s(line);
		std::string name;
		char delim;
		std::string value;

		if(line.empty() || line[0] == '#') continue;
		else if((s >> name >> delim) && delim == '=')
		{
			std::getline(s, value);
			trim(value);
			try
			{
				Set(name, value); 
			}
			catch (std::exception& e)
			{
				System::LogBas() << "Bad option value on line " << lineNumber << " What: " << e.what() << std::endl;
			}
		}
		else 
		{
			System::LogBas() << "Bad option value on line " << lineNumber << std::endl;
		}
	}
}

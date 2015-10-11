/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010  Andrey Stroganov <savthe@gmail.com>

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

#include <boost/bind.hpp>

#include "webuiserver.hpp"
#include "webuiclient.hpp"
#include "system.hpp"
#include "settings.hpp"
#include "ui/tag.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "sl.hpp"

#include <android/log.h>

using FileSystem::Exists;

WebUIServer::WebUIServer(ba::io_service& io):
	acceptor_(io, ba::ip::tcp::v4())
{
	prefs_.sorting.Update(Ui::Tags::sres, Ui::Tags::sources);
}

Ui::Prefs& WebUIServer::GetPrefs()
{
	return prefs_;
}

void WebUIServer::Start()
{
	System::LogBas() << "Starting web ui server on port " << System::GetSettings()->webui.port << std::endl;

	FindStyle();

	const uint port = System::GetSettings()->webui.port;

	try
	{
		ba::socket_base::reuse_address reuse(true);
		acceptor_.set_option(reuse);
		acceptor_.bind(ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port));
		acceptor_.listen();
		WebUIClientPtr pNewClient(new WebUIClient);
		acceptor_.async_accept(pNewClient->Socket(), boost::bind(&WebUIServer::Accept, this, pNewClient, ba::placeholders::error));
	}
	catch(std::exception& e)
	{
		System::LogBas() << "Couldn't bind webui to port " << port << ". Check if no other application is using it." << std::endl;
		throw;
	}
}

std::string WebUIServer::GetStyleRoot() const
{
	return styleRoot_;
}

void WebUIServer::UpdateSettings()
{
	FindStyle();
}

void WebUIServer::FindStyle()
{
	styleRoot_.clear();
	const std::string style = System::GetSettings()->webui.style;
	std::set<std::string> paths;
	paths.insert(System::Home());
	paths.insert("/usr/local/share/sharelin");
	paths.insert("/usr/share/sharelin");
	paths.insert(root_directory);
#ifdef DEFAULT_ROOT
	paths.insert(std::string(DEFAULT_ROOT) + "/share/sharelin");
#endif
	for(std::set<std::string>::iterator i = paths.begin(); i != paths.end(); ++i)
	{
		const std::string s = *i + "/webui/" + style;
		System::LogBas() << "Searching style '" << style << "' in " << s << " => ";
		if(Exists(s))
		{
			styleRoot_ = s;
			System::Log() << "Found" << std::endl;
			//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Found");
			break;
		}
		else System::Log() << "Not found" << std::endl;
		//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Not found");
	}
}

void WebUIServer::Detach(WebUIClientPtr p)
{
	clients_.erase(p);
}

void WebUIServer::Accept(WebUIClientPtr p, const bs::error_code& err)
{
	if(!err)
	{
		Ip::Address addr = p->GetEndpoint().address;

		bool found = false;
		foreach(IpSegment s, System::GetSettings()->general.uisegments)
			found |= s.Contains(addr);

		if(found)
		{
			clients_.insert(p);
			p->Start();
		}
		else
		{
			System::LogBas() << "Dropping webui client connection " << addr << " (Ip not allowed)" << std::endl;
		}

		WebUIClientPtr pNewClient(new WebUIClient);
		acceptor_.async_accept(pNewClient->Socket(), boost::bind(&WebUIServer::Accept, this, pNewClient, ba::placeholders::error));
	}
}



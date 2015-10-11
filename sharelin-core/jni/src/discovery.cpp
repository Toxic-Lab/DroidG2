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

#include <vector>
#include <cassert>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "discovery.hpp"
#include "identity.hpp"
#include "hubcache.hpp"
#include "url.hpp"
#include "ip.hpp"
#include "settings.hpp"
#include "system.hpp"

#include <android/log.h>

using boost::posix_time::seconds;

Discovery::Discovery(ba::io_service& io):
	socket_(io), 
	resolver_(io), 
	message_(RESPONSE_MAX_SIZE),
	timer_(io),
	closed_(false)
{
}

void Discovery::Shutdown()
{
	if(!closed_)
	{
		timer_.cancel();
		if(socket_.is_open()) socket_.close();
		closed_ = true;
	}
}

void Discovery::Start()
{
	Load();
	ScheduleRun();
}

void Discovery::ScheduleRun()
{
	timer_.expires_from_now(seconds(1));
	timer_.async_wait(boost::bind(&Discovery::Run, shared_from_this(), _1));
}

bool Discovery::SelectGwc()
{
	address_.clear();

	std::vector<std::string> v;

	for(Caches::iterator i = caches_.begin(); i != caches_.end(); ++i)
	{
		const WebCache& c = i->second;
		if(!c.banned && System::Now() - c.timestamp > REST_TIME)
			v.push_back(c.address);
	}

	if(!v.empty()) address_ = v[rand() % v.size()];

	return !address_.empty();
}

void Discovery::Run(const bs::error_code& err)
{
	if(closed_ || err == ba::error::operation_aborted) return;

	if(err)
	{
		System::LogBas() << "Error in Discovery timer: " << err.message() << std::endl;
		System::LogBas() << "Discovery service will be disabled." << std::endl;

		closed_ = true;
		return;
	}

	if(System::GetHubCache()->Size() > 10)
	{
		ScheduleRun();
		return;
	}

	if(!SelectGwc())
	{
		LoadDefaults();
		SelectGwc();
	}
	char debug[200];

	assert(!address_.empty());
	//sprintf(debug, "Discovery: parsing %s", address_.c_str());
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);

	Url::Locator loc;
	bool tom_result = loc.FromString(address_);
	assert(tom_result);
	//sprintf(debug, "Discovery: loc debug %s %s", loc.host.c_str(), loc.uri.c_str());
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);

	if(loc.port == 0) loc.port = 80;
	loc.query = "net=gnutella2&support=1&get=1&client=" + std::string(Ident::vendor) + "&version=" + Ident::version;

	message_.consume(message_.size());
	assert(message_.size() == 0);
	std::ostream r(&message_);
	r << "GET " << loc.FormatQuery() << " HTTP/1.1\r\n";
	r << "Host: " << loc.host << "\r\n";
	r << "User-Agent: " << Ident::fullname << "\r\n";
	r << "Conection: close\r\n";
	r << "\r\n";

	System::LogBas() << "Discovery started for: " << loc.host << std::endl;
	ba::ip::tcp::resolver::query query(loc.host, boost::lexical_cast<std::string>(loc.port));
	resolver_.async_resolve(query, boost::bind(&Discovery::OnResolved, shared_from_this(), ba::placeholders::error, ba::placeholders::iterator));
	timer_.expires_from_now(seconds(20));
	timer_.async_wait(boost::bind(&Discovery::Timeout, shared_from_this(), _1));
}

void Discovery::Timeout(const bs::error_code& err)
{
	if(closed_) return;

	socket_.close();
	resolver_.cancel();

	if(!err)
	{
		System::LogBas() << "Timeout during discovery " << address_ << std::endl;
		ScheduleRun();
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogBas() << "Discovery timer failed" << std::endl;
		closed_ = true;
	}
}

void Discovery::OnResolved(const bs::error_code& err, ba::ip::tcp::resolver::iterator iEndpoint)
{
	if(closed_ || err == ba::error::operation_aborted) return;

	if(err)
	{
		System::LogBas() << "Error resolving " << address_ << std::endl;
		SessionFailed();
	}
	else
	{
		ba::ip::tcp::endpoint endpoint = *iEndpoint;
		System::LogBas() << "Resolved GWebCache ( " << address_ << " ) endpoint: " << endpoint << std::endl;
		socket_.async_connect(endpoint, boost::bind(&Discovery::OnConnected, shared_from_this(), ba::placeholders::error, ++iEndpoint));
	}
}

void Discovery::OnConnected(const bs::error_code& err, ba::ip::tcp::resolver::iterator iEndpoint)
{
	if(closed_ || err == ba::error::operation_aborted) return;

	if(err)
	{
		socket_.close();

		if(iEndpoint != ba::ip::tcp::resolver::iterator())
		{
			ba::ip::tcp::endpoint endpoint = *iEndpoint;
			socket_.async_connect(endpoint, boost::bind(&Discovery::OnConnected, shared_from_this(), ba::placeholders::error, ++iEndpoint));
		}
		else
		{
			System::LogBas() << "Couldn't connect to web cache: " << address_ << std::endl;
			SessionFailed();
		}
	}
	else
	{
		System::LogBas() << "Connected to gwc " << address_ << std::endl;
		ba::async_write(socket_, message_, boost::bind(&Discovery::OnMessageSent, shared_from_this(), ba::placeholders::error));
	}
}

void Discovery::SessionFailed()
{
	caches_[address_].banned = true;
	timer_.cancel();
	ScheduleRun();
}

void Discovery::OnMessageSent(const bs::error_code& err)
{
	if(closed_) return;
	
	if(!err)
	{
		ba::async_read(socket_, message_, ba::transfer_all(), boost::bind(&Discovery::OnResponse, shared_from_this(), _1, _2)); 
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogBas() << "Web cache ( " << address_ << " ) send error: " << err.message() << std::endl;
		SessionFailed();
	}
}

void Discovery::OnResponse(const bs::error_code& err, std::size_t length)
{
	if(closed_ || err == ba::error::operation_aborted) return;

	if(!err || err == ba::error::eof)
	{
		timer_.cancel();

		System::LogBas() << "Got reply from gwc " << address_ << std::endl;
		try { HandleMessage(); }
		catch(std::exception& r)
		{
			System::LogBas() << "Couldn't handle web cache ( " << address_ << ") reply => " << r.what() << std::endl;
			caches_[address_].banned = true;
		}

		if(!err) socket_.close();

		ScheduleRun();

	}
	else
	{
		System::LogBas() << "Data receive error from web cache ( " << address_ << " Error: " << err.message() << std::endl;
		SessionFailed();
	}
}

void Discovery::HandleMessage()
{
	std::istream r(&message_);
	std::string line;
	r >> line; //skip version
	r >> line; //code
	if(line != "200")
	{
		std::string comment;
		std::getline(r, comment);
		throw std::runtime_error("Bad reply code: " + line + " " + comment);
	}

	while(std::getline(r, line) && line != "\r") /* =) */ ;

	typedef std::map<Ip::Endpoint, uint> Endpoints;
	typedef std::map<std::string, uint> WebCaches;

	Endpoints hubs;
	WebCaches webcaches;

	bool g2support = false;
	uint32 period = 0;
	while(std::getline(r, line))
	{
		boost::replace_all(line, "|", " ");
		std::stringstream s(line);
		char prefix;
		s >> prefix;
		switch(prefix)
		{
			case 'I': case'i':
			{
				std::string tag;
				std::string val;
				if(s >> tag >> val)
				{
					if(boost::iequals(tag, "support") && boost::iequals(val, "gnutella2"))
						g2support = true;
					else if(boost::iequals(tag, "access") && boost::iequals(val, "period"))
						s >> period;
				}
				break;
			}
			case 'H': case 'h':
			{
				std::string address;
				std::time_t seen = 0;
				if(s >> address)
				{
					s >> seen;
					seen = seen <= System::Now() ? seen : System::Now();
					Ip::Endpoint ep;
					if(ep.FromString(address))
						hubs[ep] = System::Since(seen);

				}
				break;
			}
			case 'U': case 'u':
			{
				std::string address;
				uint seen = 0;
				if(s >> address)
				{
					s >> seen;
					webcaches[address] =  System::Now() - seen;
				}
			}
		}
	}

	if(g2support)
	{
		System::LogBas() << "Discovered " << hubs.size() << " hub(s) and " << webcaches.size() << " web cache(s)" << std::endl;
		for(Endpoints::iterator i = hubs.begin(); i != hubs.end(); ++i)
			System::GetHubCache()->Touch(i->first, i->second);
		for(WebCaches::iterator i = webcaches.begin(); i != webcaches.end(); ++i)
			AddCache(i->first);
	}
	else
		caches_[address_].banned = true;
}	

void Discovery::LoadDefaults()
{
	System::LogAdv() << "Loading default webcaches" << std::endl;

	caches_.clear();

	AddCache("http://cache.trillinux.org/g2/bazooka.php");
	AddCache("http://gwc2.wodi.org/skulls.php");
	AddCache("http://gwc.frodoslair.net/beacon/gwc.php");
	AddCache("http://htmlhell.com/ ");
	AddCache("http://dkac.trillinux.org/dkac/dkac.php");
	AddCache("http://silvers.zyns.com/gwc/dkac.php"); //home, sweet home =)
	AddCache("http://gwc.frodoslair.net/beacon/gwc ");
	AddCache("http://gwc.dyndns.info:28960/gwc.php ");
	AddCache("http://silvers.zyns.com/gwc/dkac.php");
	AddCache("http://gwc.marksieklucki.com/skulls.php");
	AddCache("http://karma.cloud.bishopston.net:33559/");
}

void Discovery::Load()
{
	
	std::ifstream f((System::Home() + "/discovery.dat").c_str());
	std::string line;
	int goodCount = 0;
	while(std::getline(f, line))
	{
		if(line.empty() || line[0] == '#') 
			continue;
		
		char prefix;
		WebCache gwc;
		std::stringstream s(line);

		if(s >> prefix >> gwc.address)
		{
			if(!(s >> gwc.timestamp) && prefix == 'A')
				gwc.timestamp = time(0);
			else if(prefix == 'B' || prefix == 'X')
				gwc.banned = true;
			if(!gwc.banned)
				++goodCount;
			AddCache(gwc);
		}
	}

	if(goodCount == 0)
	{
		caches_.clear();
		LoadDefaults();
	}
}

void Discovery::AddCache(const std::string& address)
{
	WebCache gwc;
	gwc.address = address;
	//gwc.timestamp = System::Now();
	AddCache(gwc);
}

void Discovery::AddCache(const Discovery::WebCache& gwc)
{
	Caches::iterator i = caches_.find(gwc.address);
	if(i != caches_.end() && i->second.banned){
		return;
	}
	Url::Locator loc;
	if(loc.FromString(gwc.address))
		caches_[gwc.address] = gwc;
}

void Discovery::Save()
{
	std::ofstream f((System::Home() + "/discovery.dat").c_str());
	f << "#Web caches. To add a cache use A prefix followed by space and cache address. For example:" << std::endl;
	f << "#A http://some_site/gwc.php" << std::endl;
	f << "#Third parameter is a timestamp. Don't change it manually." << std::endl;
	f << "#Legend: (A)live, (B)anned, (U)ntested." << std::endl;
	const time_t too_old = 60 * 60 * 24 * 7;
	for(Caches::iterator i = caches_.begin(); i != caches_.end(); ++i)
	{
		WebCache gwc = i->second;
		
		if(gwc.banned) f << "B ";
		else if(System::Now() - gwc.timestamp  > too_old) f << "U ";
		else f<< "A ";
		
		f << gwc.address << " ";
		f << gwc.timestamp << std::endl;
	}
}

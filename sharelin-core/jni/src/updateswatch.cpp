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

#include "updateswatch.hpp"
#include "identity.hpp"
#include "system.hpp"

using boost::posix_time::seconds;
using boost::posix_time::hours;

namespace {

const std::size_t BUFFER_SIZE = 32 * 1024;
const char updatesHost[] = "sharelin.sourceforge.net";

} //namespace 

UpdatesWatch::UpdatesWatch(ba::io_service& io):
	socket_(io),
	resolver_(io),
	timer_(io),
	buffer_(BUFFER_SIZE),
	closed_(false),
	birthday_(0)
{
}

bool UpdatesWatch::NewVersion() const
{
	return birthday_ > Ident::birthday;
}

void UpdatesWatch::Start()
{
	timer_.expires_from_now(seconds(1));
	timer_.async_wait(boost::bind(&UpdatesWatch::Run, shared_from_this(), _1));
}

void UpdatesWatch::ScheduleRun()
{
	timer_.expires_from_now(hours(1));
	timer_.async_wait(boost::bind(&UpdatesWatch::Run, shared_from_this(), _1));
}

void UpdatesWatch::Run(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		ba::ip::tcp::resolver::query query(updatesHost, "80");
		resolver_.async_resolve(query, boost::bind(&UpdatesWatch::OnResolved, shared_from_this(), ba::placeholders::error, ba::placeholders::iterator));
		timer_.expires_from_now(seconds(10));
		timer_.async_wait(boost::bind(&UpdatesWatch::Timeout, shared_from_this(), _1));

	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogDev() << "UpdatesWatch::Run error => " << err.message() << std::endl;
	}
}

void UpdatesWatch::Timeout(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		resolver_.cancel();
		socket_.close();
		ScheduleRun();
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogDev() << "UpdatesWatch::Timeout error" << std::endl;
	}
}

void UpdatesWatch::OnResolved(const bs::error_code& err, ba::ip::tcp::resolver::iterator iep)
{
	if(closed_) return;
	
	if(!err)
	{
		ba::ip::tcp::endpoint endpoint = *iep;
		System::LogDev() << "Resolved updates host: " << endpoint << std::endl;
		socket_.async_connect(endpoint, boost::bind(&UpdatesWatch::OnConnected, shared_from_this(), ba::placeholders::error, ++iep));
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogBas() << "Couldn't resolve " << updatesHost << std::endl;
		timer_.cancel();
		ScheduleRun();
	}
}

void UpdatesWatch::OnConnected(const bs::error_code& err, ba::ip::tcp::resolver::iterator iep)
{
	if(closed_) return;

	if(!err)
	{
		System::LogAdv() << "Connected to updates host. Requesting information." << std::endl;
		std::ostream r(&buffer_);
		r << "GET /version.dat HTTP/1.0\r\n";
		r << "User-Agent: " << Ident::fullname << "\r\n";
		r << "Accept: */*\r\n";
		r << "Host: " << updatesHost << "\r\n";
		r << "Connection: close" << "\r\n";
		r << "\r\n";
		ba::async_write(socket_, buffer_, boost::bind(&UpdatesWatch::OnWrite, shared_from_this(), _1));
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogAdv() << "Couldn't connect to updates host => " << err.message() << std::endl;
		timer_.cancel();
		ScheduleRun();
	}
}

void UpdatesWatch::OnWrite(const bs::error_code& err)
{
	if(!err)
	{
		ba::async_read(socket_, buffer_, ba::transfer_all(), boost::bind(&UpdatesWatch::OnReply, shared_from_this(), _1, _2));
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogDev() << "UpdatesWatch::OnWrite error => " << err.message() << std::endl;
		timer_.cancel();
		ScheduleRun();
	}
}

void UpdatesWatch::OnReply(const bs::error_code& err, std::size_t nbytes)
{
	if(!err || err == ba::error::eof)
	{
		System::LogDev() << "Got reply from update server" << std::endl;

		std::istream r(&buffer_);
		std::string line;
		r >> line;
		r >> line;
		if(line == "200")
		{
			birthday_ = 0;
			tag_.clear();
			link_.clear();
			text_.clear();

			while(std::getline(r, line) && line != "\r");
			if(r >> birthday_ >> tag_ >> link_)
			{
				if(NewVersion()) System::LogBas() << "New version is available: " << Tag() << std::endl;
				while(std::getline(r, line))
				{
					if(line.empty()) continue;
					if(NewVersion()) System::LogBas() << line << std::endl;
					text_ += line + '\n';
				}
			}
		}
		else System::LogAdv() << "Bad reply from update server: " << line << std::endl;

		socket_.close();
		timer_.cancel();
		ScheduleRun();
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogBas() << "Couldn't get reply from updates server => " << err.message() << std::endl;
		timer_.cancel();
		ScheduleRun();
	}
}

void UpdatesWatch::Shutdown()
{
	socket_.close();
	timer_.cancel();
}


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
#include <boost/algorithm/string.hpp>

#include "g2handshake.hpp"
#include "system.hpp"
#include "hubcache.hpp"
#include "identity.hpp"
#include "g2net.hpp"

using boost::iequals;

namespace {

void ExtractHubs(const std::string& line)
{
	std::stringstream s(line);
	std::string hub;
	while(std::getline(s, hub, ','))
	{
		std::size_t pos = hub.find(' ');
		if(pos == std::string::npos) continue;

		Ip::Endpoint endpoint;
		if(endpoint.FromString(hub.substr(0, pos)))
		{
			System::GetHubCache()->Touch(endpoint);
		}
		//FIXME timestamp
	}
}

}

G2Handshake::G2Handshake():
	pSocket_(new ba::ip::tcp::socket(System::GetIo())),
	timer_(System::GetIo()),
	buffer_(64 * 1024),
	connecting_(false),
	closed_(false),
	alive_(false)
{
}

void G2Handshake::Connect(Ip::Endpoint endpoint)
{
	System::LogAdv() << "Connecting to hub " << endpoint << std::endl;

	connecting_ = true;
	endpoint_ = endpoint;

	timer_.expires_from_now(boost::posix_time::seconds(5));
	timer_.async_wait(boost::bind(&G2Handshake::OnTimeout, shared_from_this(), ba::placeholders::error));

	ba::ip::tcp::endpoint asioEndpoint(ba::ip::address_v4(endpoint.address.Reversed()), endpoint.port);
	pSocket_->async_connect(asioEndpoint, boost::bind(&G2Handshake::OnConnected, shared_from_this(), ba::placeholders::error));
}

void G2Handshake::OnConnected(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		connecting_ = false;

		System::LogAdv() << "Connected to " << endpoint_ << std::endl;

		std::ostream r(&buffer_);
		r << "GNUTELLA CONNECT/0.6\r\n";

		const Ip::Endpoint self = System::SelfEndpoint();
		if(!self.Empty()) r << "Listen-IP: " << self << "\r\n";

		r << "Remote-IP: " << endpoint_.address << "\r\n";
		r << "User-Agent: " << Ident::fullname << "\r\n";
		r << "Accept: application/x-gnutella2\r\n";
		r << "X-Hub: False\r\n";
		r << "X-Hub-Needed: True\r\n";
		r << "\r\n";

		ba::async_write(*pSocket_, buffer_, boost::bind(&G2Handshake::OnGreetingSent, shared_from_this(), ba::placeholders::error));
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogDev() << "G2Handshake::OnConnected " << endpoint_ << " error => " << err.message() << std::endl;
		DetachMe();
	}
}

void G2Handshake::OnGreetingSent(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		ba::async_read_until(*pSocket_, buffer_, "\r\n\r\n", 
				boost::bind(&G2Handshake::OnGreetingResponse, shared_from_this(), ba::placeholders::error));
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogDev() << "G2Handshake::OnGreetingSent " << endpoint_ << " error => " << err.message() << std::endl;
		DetachMe();
	}
}

void G2Handshake::OnGreetingResponse(const bs::error_code& err)
{
	if(closed_) return;
	
	if(err)
	{
		if(err == ba::error::operation_aborted) return;

		System::LogDev() << "G2Handshake::OnGreetingResponse " << endpoint_ << " error => " << err.message() << std::endl;
		DetachMe();
		return;
	}

	try { HandleReply(); }
	catch(std::exception& e)
	{
		System::LogAdv() << "Connect to hub " << endpoint_ << " failed => " << e.what() << std::endl;
		DetachMe();
		return;
	}
}
	
void G2Handshake::HandleReply()
{
	std::istream r(&buffer_);
	std::string line;
	uint code;
	std::string message;

	if(!(r >> line >> code))
		throw std::runtime_error("First line parsing error");
	if(!iequals(line, "GNUTELLA/0.6"))
		throw std::runtime_error("Not G2 host");
	alive_ = true;
	std::getline(r, message);

	while(std::getline(r, line) && line != "\r")
	{
		std::stringstream s(line);
		std::string header;
		std::string value;
		s >> header;
		std::getline(s, value);

		if(iequals(header, "Remote-IP:"))
		{
			selfAddress_.FromString(value);
		}
		else if(iequals(header, "X-Try-Hubs:"))
			ExtractHubs(value);
	}

	if(code == 200)
	{
		std::ostream r(&buffer_);
		r << "GNUTELLA/0.6 200 OK\r\n";
		r << "Content-Type: application/x-gnutella2\r\n";
		r << "X-Hub: False\r\n";
		r << "\r\n";
		ba::async_write(*pSocket_, buffer_, boost::bind(&G2Handshake::OnG2Connected, shared_from_this(), ba::placeholders::error));
	}
	else throw std::runtime_error(message);
}

void G2Handshake::OnG2Connected(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		timer_.cancel();
		System::GetNetwork()->OnHubConnected(this);
	}
}

void G2Handshake::OnTimeout(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		System::LogAdv() << "Connection to hub " << endpoint_ << " timed out" << std::endl;
		pSocket_->close();
		closed_ = true;
		DetachMe();
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogDev() << "G2Handshake::OnTimer " << endpoint_ << " error => " << err.message() << std::endl;
	}
}

void G2Handshake::Close()
{
	if(!closed_)
	{	
		pSocket_->close();
		timer_.cancel();
		closed_ = true;
	}
}

void G2Handshake::DetachMe()
{
	Close();
	boost::shared_ptr<G2Handshake> me = shared_from_this();
	System::GetNetwork()->Detach(me);
}


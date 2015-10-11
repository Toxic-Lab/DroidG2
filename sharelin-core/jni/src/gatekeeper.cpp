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

#include "gatekeeper.hpp"
#include "system.hpp"
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include "ip.hpp"
#include "downloadmanager.hpp"
#include "download.hpp"
#include "uploadmanager.hpp"
#include <sstream>
#include "g2net.hpp"

using boost::iequals;

Gatekeeper::Gatekeeper():
	pSock_(new ba::ip::tcp::socket(System::GetIo())),
	timer_(System::GetIo()),
	buffer_(BUFFER_SIZE),
	closed_(false)
{
	message_.reserve(BUFFER_SIZE);
}

void Gatekeeper::Start()
{
	try
	{
		timer_.expires_from_now(boost::posix_time::seconds(5));
		timer_.async_wait(boost::bind(&Gatekeeper::Timeout, shared_from_this(), ba::placeholders::error));
		ba::ip::tcp::endpoint ep = pSock_->remote_endpoint();
		endpoint_.address.value = ep.address().to_v4().to_ulong();
		endpoint_.address.Reverse();
		endpoint_.port = ep.port();
		pSock_->async_read_some(ba::buffer(buffer_), boost::bind(&Gatekeeper::OnData, shared_from_this(), _1, _2));
		System::LogAdv() << "Accepted incomming connection from " << endpoint_ << std::endl;
	}
	catch(std::exception& e)
	{
		System::LogAdv() << "Couldn't start gatekeeper session => " << e.what() << std::endl;
		closed_ = true;
		System::GetNetwork()->Detach(shared_from_this());
	}
}

void Gatekeeper::Push(Ip::Endpoint endpoint)
{
	timer_.expires_from_now(boost::posix_time::seconds(5));
	timer_.async_wait(boost::bind(&Gatekeeper::Timeout, shared_from_this(), ba::placeholders::error));
	endpoint_ = endpoint;
	ba::ip::tcp::endpoint asioEndpoint(ba::ip::address_v4(endpoint.address.Reversed()), endpoint.port);
	pSock_->async_connect(asioEndpoint, boost::bind(&Gatekeeper::OnConnected, shared_from_this(), ba::placeholders::error));
	System::LogAdv() << "Connecting by push request to " << endpoint_ << std::endl;
}

void Gatekeeper::OnConnected(const bs::error_code& err)
{
	if(closed_) return;

	if(!err) 
	{
		System::LogBas() << "Connected by push to " << endpoint_ << std::endl;
		std::string greet = "GIV 0:";
		const GUID guid = System::Guid();
		Conv::Hex::Encode(guid.begin(), guid.end(), std::back_inserter(greet));
		greet += "/\r\n";
		buffer_.assign(greet.begin(), greet.end());
		ba::async_write(*pSock_, ba::buffer(buffer_), boost::bind(&Gatekeeper::OnGreetingSent, shared_from_this(), _1));
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogDev() << "Gatekeeper::OnConnected " << endpoint_ << " error => " << err.message() << std::endl;
		DetachMe();
	}
}

void Gatekeeper::OnGreetingSent(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		pSock_->async_read_some(ba::buffer(buffer_), boost::bind(&Gatekeeper::OnData, shared_from_this(), _1, _2));
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogDev() << "Gatekeeper::OnGreetingSent " << endpoint_ << " error => " << err.message() << std::endl;
		DetachMe();
	}
}

void Gatekeeper::Close()
{
	if(!closed_)
	{
		closed_ = true;
		timer_.cancel(); 
		pSock_->close(); 
	}
}

void Gatekeeper::Timeout(const bs::error_code& err)
{
	if (!closed_ && err != ba::error::operation_aborted)
	{
		System::LogAdv() << "Gatekeeper " << endpoint_ << " timed out" << std::endl;
		closed_ = true;
		pSock_->close(); 
		System::GetNetwork()->Detach(shared_from_this());
	}
}

void Gatekeeper::DetachMe()
{
	timer_.cancel();
	System::GetNetwork()->Detach(shared_from_this());
}

void Gatekeeper::Handle()
{
	std::stringstream r(message_);
	std::string s;

	r >> s;

	if( iequals( s, "GIV" ) )
	{
		std::getline(r, s);
		const std::size_t pos = s.find(':');
		std::string hex = s.substr(pos + 1, 2 * 16);
		GUID guid;
		assert(hex.size() == 32);
		Conv::Hex::Decode(hex.begin(), hex.end(), guid.begin());

		std::vector< DownloadPtr > v;
		System::GetDownloadMgr()->Dump(std::back_inserter(v));
		for(uint i = 0; i < v.size(); ++i)
			if(v[i]->HandleGIV(this, guid)) 
				break;
		return;
	}
	else if(iequals(s, "GET"))
	{
		if(message_.find("\r\n\r\n") == std::string::npos)
			throw MessageIncomplete();
		System::GetUploadMgr()->Accept(this);
	}
	else
		throw std::runtime_error("Unhandled");

}

void Gatekeeper::OnData(const bs::error_code& err, std::size_t length)
{
	if(closed_) return;

	if(!err) 
	{
		std::copy(buffer_.begin(), buffer_.begin() + length, std::back_inserter(message_));
		try { Handle(); }
		catch(MessageIncomplete& )
		{
			pSock_->async_read_some(ba::buffer(buffer_), boost::bind(&Gatekeeper::OnData, shared_from_this(), _1, _2));
			return;
		}
		catch( std::exception& e ) 
		{
			System::LogBas() << "Problem in gatekeeper " << endpoint_ << " => " << e.what() << std::endl;
		}

	}

	if(err != ba::error::operation_aborted)
		DetachMe();
}


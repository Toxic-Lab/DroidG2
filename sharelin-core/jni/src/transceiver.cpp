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

#include <boost/bind.hpp>

#include "transceiver.hpp"
#include "g2packet.hpp"
#include "g2net.hpp"
#include "system.hpp"
#include "settings.hpp"

Transceiver::Transceiver(ba::io_service& io):
	socket_(io, ba::ip::udp::v4()),
	timer_(io, boost::posix_time::seconds(1)),
	sending_(false),
	throttle_(false)
{
}

void Transceiver::Start(uint16 port)
{
	traffin_.SetShare(System::GetSettings()->net.maxBwIn * 0.05);
	timer_.async_wait(boost::bind(&Transceiver::OnTimer, this));
	if(System::GetSettings()->net.directAble != Settings::NO)
	{
		socket_.bind(ba::ip::udp::endpoint(ba::ip::udp::v4(), port));
		socket_.async_receive_from(ba::buffer(receiveBuffer_, BUFFER_SIZE), sender_, 
			boost::bind(&Transceiver::OnReceived, this, ba::placeholders::error, ba::placeholders::bytes_transferred));
	}
}

void Transceiver::OnReceived(const bs::error_code& err, std::size_t bytesReceived)
{
	if(err == ba::error::operation_aborted) return;

	Ip::Endpoint endpoint;
	if(err) System::LogBas() << "Transceiver::OnReceived error => " << err.message() << std::endl;
	else
	{
		endpoint = Ip::Endpoint(sender_.address().to_v4().to_ulong(), sender_.port());
		endpoint.address.Reverse();
	}

	udp::Header header;
	try
	{
		if(bytesReceived < udp::HEADER_SIZE) throw std::runtime_error("Packet is too small");
		if(!header.Unpack(receiveBuffer_, bytesReceived)) throw std::runtime_error("Bad header");
	}
	catch (std::exception& e)
	{
		System::LogAdv() << "Error in transceiver ";
		if(!endpoint.Empty()) System::Log() << "from " << endpoint << " ";
		System::Log() << "=> " << e.what() << std::endl;

		StartReceive();

		return;
	}

	if(bytesReceived == udp::HEADER_SIZE || header.totalParts == 0)
		departure_.Acknowlege(header.id, header.partNumber);
	else 
	{
		if(header.acknowledge)
		{
			udp::Header ackHeader = header;
			ackHeader.deflate = false;
			ackHeader.acknowledge =false;
			ackHeader.totalParts = 0;
			departure_.Put(ackHeader, endpoint);
		}

		try
		{
			G2::Packet pk;
			if(arrival_.Put(receiveBuffer_, bytesReceived, endpoint, pk))
				System::GetNetwork()->OnPacket(pk, endpoint, Ip::Protocol::UDP);
		}
		catch(std::exception& e) 
		{
			System::LogBas() << "Couldn't handle udp packet from " << endpoint << " => " << e.what() << std::endl;
		}
	}

	if(traffin_.Use(Ip::Endpoint(), bytesReceived) == 0)
		System::GetTraffIn()->Use(Ip::Endpoint(), bytesReceived);

	StartReceive();
}

void Transceiver::StartReceive()
{
	throttle_ = false;
	if(traffin_.Use(Ip::Endpoint(), 1) || System::GetTraffIn()->Use(Ip::Endpoint(), 1))
		socket_.async_receive_from(ba::buffer(receiveBuffer_, BUFFER_SIZE), sender_, 
			boost::bind(&Transceiver::OnReceived, this, ba::placeholders::error, ba::placeholders::bytes_transferred));
	else throttle_ = true;
}

void Transceiver::Send(const G2::Packet& packet, Ip::Endpoint endpoint)
{
	departure_.Put(packet, endpoint);	
}

void Transceiver::OnTimer()
{
	if(!sending_) Dispatch();
	if(throttle_) StartReceive();

	arrival_.PurgeOld();

	traffin_.OnTimer(System::Now());

	timer_.expires_from_now(boost::posix_time::seconds(1));
	timer_.async_wait(boost::bind(&Transceiver::OnTimer, this));
}

void Transceiver::Dispatch()
{
	assert(!sending_);

	Ip::Endpoint endpoint;
	const std::size_t size = departure_.Get(sendBuffer_, endpoint);
	if(size > 0)
	{
		sending_ = true;
		socket_.async_send_to(ba::buffer(sendBuffer_, size), ba::ip::udp::endpoint(ba::ip::address_v4(endpoint.address.Reversed()), endpoint.port), 
				boost::bind(&Transceiver::OnSent, this, ba::placeholders::error, ba::placeholders::bytes_transferred));
	}
}

void Transceiver::OnSent(const bs::error_code& err, std::size_t bytesSent)
{
	if(err) System::LogAdv() << "Problem while data sending in transceiver => " << err.message() << std::endl;

	sending_ = false;
	Dispatch();
}

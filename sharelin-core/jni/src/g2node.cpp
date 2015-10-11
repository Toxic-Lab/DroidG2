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

#include <stdexcept>
#include <cassert>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include "g2node.hpp"
#include "g2net.hpp"
#include "identity.hpp"
#include "g2packet.hpp"
#include "identity.hpp"
#include "staticname.hpp"
#include "system.hpp"
#include "hubcache.hpp"
#include "sharemanager.hpp"
#include "queryhashtable.hpp"
#include "zlib.hpp"
#include "g2handshake.hpp"

namespace {
	const std::time_t ALIVE_TIMEOUT = 20;
}

G2Node::G2Node(G2Handshake* pHandshake):
	pSocket_(pHandshake->GetSocket()), 
	timestamp_(0),
	periodLNI_(Time::Minutes(1)),
	periodPing_(Time::Seconds(15)),
	periodQHT_(Time::Minutes(5)),
	qhtTime_(0),
	closed_(false)
{
	myInfo_.endpoint = pHandshake->GetEndpoint();
}
	
void G2Node::Send(const G2::Packet& packet)
{
	dispatchQueue_.push(packet);
	if(packetBuf_.empty())
		SendPackets();
}

void G2Node::SendPackets()
{
	assert(packetBuf_.empty());
	G2::Packet& pk = dispatchQueue_.front();
	G2::Reader r(pk);
	//System::LogBas() << "Node " << myInfo_.endpoint << " is sending " << StaticName::ToString(r.Type()) << std::endl;
	packetBuf_.resize(pk.Size());
	std::memcpy(&packetBuf_[0], pk.Bytes(), pk.Size());
	ba::async_write(*pSocket_, ba::buffer(packetBuf_), boost::bind(&G2Node::OnPacketSent, shared_from_this(), ba::placeholders::error));
}

void G2Node::OnPacketSent(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		packetBuf_.clear();
		dispatchQueue_.pop();
		if(!dispatchQueue_.empty()) SendPackets();
	}
	else if(err != ba::error::operation_aborted) 
	{
		System::LogDev() << "G2Node::OnPacketSent " << myInfo_.endpoint.ToString() << " " << err.message() << std::endl;
		DetachMe();
	}
}

void G2Node::Ping()
{
	System::LogDev() << "Pinging neighboring hub " << myInfo_.endpoint << std::endl;

	G2::Packet pi;
	G2::Writer w(pi);
	w.Begin(G2::PI);
	w.Close(G2::PI);

	Send(pi);
}

void G2Node::Start()
{
	timestamp_ = System::Now();
	UpdateQHT();
	ReadPacket();
}

void G2Node::ReadPacket()
{
	g2buffer_.resize(1);
	rawPacket_.clear();
	ba::async_read(*pSocket_, ba::buffer(g2buffer_), ba::transfer_all(), boost::bind(&G2Node::OnControlByte, shared_from_this(), ba::placeholders::error));
}

G2Node::~G2Node()
{
}

void G2Node::OnControlByte(const bs::error_code& err)
{
	if(closed_) return;

	assert(g2buffer_.size() == 1);
	if(!err)
	{
		rawPacket_.push_back(g2buffer_[0]);
		G2::ControlByte cb(g2buffer_[0]);
		g2buffer_.resize(cb.lengthSize + cb.nameSize);
		ba::async_read(*pSocket_, ba::buffer(g2buffer_), ba::transfer_all(), 
				boost::bind(&G2Node::OnPacketHeader, shared_from_this(), ba::placeholders::error));
	}
	else if(err != ba::error::operation_aborted) 
	{
		System::LogDev() << "G2Node::OnControlByte " << myInfo_.endpoint.ToString() << " " << err.message() << std::endl;
		DetachMe();
	}
}

void G2Node::OnPacketHeader(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		assert(g2buffer_.size() > 0);
		rawPacket_.insert(rawPacket_.end(), g2buffer_.begin(), g2buffer_.end());
		G2::Header hd;
		try{ hd.Decode(&rawPacket_[0], rawPacket_.size()); }
		catch( std::exception& e)
		{
			ReadPacket();
			return;
		}

		if(hd.dataSize > 0)
		{
			g2buffer_.resize(hd.dataSize);
			ba::async_read(*pSocket_, ba::buffer(g2buffer_), ba::transfer_all(), 
					boost::bind(&G2Node::OnPayload, shared_from_this(), ba::placeholders::error));
		}
		else HandlePacket();
	}
	else if(err != ba::error::operation_aborted) 
	{
		System::LogDev() << "G2Node::OnPacketHeader " << myInfo_.endpoint.ToString() << " " << err.message() << std::endl;
		DetachMe();
	}
}

void G2Node::OnPayload(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		rawPacket_.insert(rawPacket_.end(), g2buffer_.begin(), g2buffer_.end());
		HandlePacket();
	}
	else if(err != ba::error::operation_aborted) 
	{
		System::LogDev() << "G2Node::OnPayload " << myInfo_.endpoint.ToString() << " " << err.message() << std::endl;
		DetachMe();
	}
}

void G2Node::HandlePacket()
{
	try
	{
		G2::Packet pk;
		pk.GetBuffer()->assign(&rawPacket_[0], rawPacket_.size());
		//std::vector<byte> test;
		//pk.Pack(test);

		G2::Reader r(pk);
		if(r.Type() == G2::LNI)
		{
			while(r.Next())
			{
				switch(r.Type())
				{
					case G2::GU:
						r.Read(myInfo_.guid.Bytes(), 16);
						break;
				}
			}

		}

		System::GetNetwork()->OnPacket(pk, myInfo_.endpoint, Ip::Protocol::TCP);

		timestamp_ = System::Now();
	}
	catch(std::exception& e)
	{
		System::LogAdv() << "Host " << myInfo_.endpoint.ToString() << " sent a corrupted packet: " << e.what() << std::endl;
	}

	ReadPacket();
}

void G2Node::UpdateLNI()
{
	G2::Packet lni;
	G2::Writer w(lni);

	w.Begin(G2::LNI);

	w.Begin(G2::NA);
	const Ip::Endpoint self = System::SelfEndpoint();
	w.Pod(self.address.value);
	w.Pod(self.port);
	w.Close(G2::NA);

	w.Begin(G2::GU);
	w.Write(System::Guid().Get(), 16);
	w.Close(G2::GU);

	w.Begin(G2::V);
	w.Pod(Ident::vid);
	w.Close(G2::V);

	w.Close(G2::LNI);

	Send(lni);
}

void G2Node::ResetQHT()
{
	const Share::QueryHashTable& qht = System::GetShareMgr()->Qht();

	G2::Packet pk;
	G2::Writer w(pk);

	w.Begin(G2::QHT);
	w.Pod(char(0));
	w.Pod(qht.SizeInBits());
	w.Pod(char(1));
	w.Close(G2::QHT);
	Send(pk);
}

void G2Node::SendQHT()
{
	const Share::QueryHashTable& qht = System::GetShareMgr()->Qht();
	const uint packetSize = 1024;
	std::vector<char> inverted( qht.Size() );
	uint count = 0;
	for(uint i = 0; i < qht.Size(); ++i)
	{
		if(*(qht.Bytes() + i))
			++count;
		inverted.at(i) = 0xFF - *(qht.Bytes() + i);
	}
	//std::cout << count << "/" << qht.Size() << std::endl;

	std::vector<char> compressed( ZLib::CompressBound( qht.Size() ) );
	uint compressedSize;
	try { compressedSize = ZLib::Compress( &inverted[0], qht.Size(), &compressed[0], compressed.size() ); }
	catch (std::exception& err)
	{
		System::LogBas() << "QHT compression error: " << err.what() << std::endl;
		return;
	}
	//std::cout << "QHT SIZE: " << qht.Size() << " COMPRESSED: " << compressedSize << std::endl;

	uint8 total = compressedSize / packetSize + (compressedSize % packetSize ? 1 : 0);

	assert(compressedSize / packetSize + 1 < 256);

	uint offset = 0;

	for( uint8 i = 0; i < total; ++i)
	{
		G2::Packet pk;
		G2::Writer w(pk);

		w.Begin(G2::QHT);
		w.Pod(char(1)); //command
		w.Pod(char(i + 1)); //fragment number
		w.Pod(char(total));
		w.Pod(char(1)); //compressed
		w.Pod(char(1)); //g2

		const uint len = std::min(packetSize, compressedSize - offset);
		w.Write( &compressed[offset], len );
		offset += len;

		w.Close(G2::QHT);
		Send(pk);
	}
}

void G2Node::UpdateQHT()
{
	//std::cout << "Updating QHT on " << myInfo_.endpoint << std::endl;
	//std::cout << qhtTime_ << " " << app_.pShareMan->QhtTime() << std::endl;
	if(qhtTime_ == System::GetShareMgr()->QhtTime()) return;
	qhtTime_ = std::time(0);

	ResetQHT();
	SendQHT();

}


void G2Node::OnTimer()
{
	if(System::Since(timestamp_) > ALIVE_TIMEOUT)
	{
		System::LogBas() << "Hub " << myInfo_.endpoint << " timeout. Disconnecting." << std::endl;
		Close();
		DetachMe();
		return;
	}

	if(periodLNI_.Set()) UpdateLNI();

	if(periodQHT_.Set()) UpdateQHT();

	if(periodPing_.Set()) Ping();
}

G2::NodeInfo G2Node::Info()
{
	return myInfo_;
}

void G2Node::Close()
{
	if(!closed_)
	{
		pSocket_->close();
		closed_ = true;
	}
}
	
void G2Node::DetachMe()
{
	Close();
	boost::shared_ptr< G2Node > me = shared_from_this();
	System::GetNetwork()->Detach(me);
}

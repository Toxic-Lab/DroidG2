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

#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include "uploader_impl.hpp"
#include "system.hpp"
#include "uploadmanager.hpp"
#include "sharemanager.hpp"
#include "filemanager.hpp"
#include "identity.hpp"
#include "settings.hpp"
#include "gatekeeper.hpp"
#include "trafmanager.hpp"
#include "security.hpp"

using boost::istarts_with;
using boost::starts_with;
using boost::trim_copy;
using boost::iequals;

namespace {

const uint BULK_SIZE = 4096;
const uint MIN_AVERAGE = 10;
const std::time_t TRANSFER_TIMEOUT = BULK_SIZE / MIN_AVERAGE;

}

UploaderImp::UploaderImp(const Gatekeeper* pGatekeeper):
	pSock_(pGatekeeper->GetSocketPtr()),
	endpoint_(pGatekeeper->Address()),
	totalSent_(0),
	offset_(0),
	bwlimit_(false),
	keepAlive_(false),
	request_(16 * 1024),
	status_(IDLE),
	transferStarted_(0),
	queuePos_(0),
	queueSize_(0),
	queued_(0)
{
	timestamp_ = time(0);
	std::ostreambuf_iterator<char> out(&request_);
	pGatekeeper->CopyRequest(out);
}

UploaderImp::~UploaderImp()
{
	assert(status_ == CLOSED);
}

void UploaderImp::Start()
{
	timestamp_ = System::Now();
	HandleRequest();
}

uint UploaderImp::AvrRate() const
{
	if(status_ == TRANSFER)
	{
		const std::time_t duration = System::Now() - transferStarted_;
		return duration == 0 ? offset_ : offset_ / duration;
	}

	return 0;
}

void UploaderImp::OnRequest( const bs::error_code& err )
{
	if(status_ == CLOSED) return;

	if( !err )
	{
		HandleRequest();
		timestamp_ = System::Now();
	}
	else if( err != ba::error::operation_aborted )
	{
		System::LogDev() << "Uploader::OnRequest " << endpoint_ << " error => " << err.message() << std::endl;
		DetachMe();
	}
}

void UploaderImp::TranslateRequest()
{
	std::istream s( &request_ );

	std::string line;

	s >> line; //method

	if( line == "GET" ) method_ = GET;
	else if( line == "HEAD" ) method_ = HEAD;
	else throw Unhandled(400, "Unknown method" );

	s >> line; //object
	if( !starts_with( line, "/uri-res/N2R?" ) ) 
		throw Unhandled(400, "Requested uri type is not supported");

	std::string last = urn_;
	urn_ = line.substr(line.find('?') + 1);

	fileInfo_ = System::GetShareMgr()->GetByUrn( urn_ ); 

	if(last != urn_)
	{
		System::LogBas() << "Host " << endpoint_ << " requested file: " << fileInfo_.Name() << std::endl;
		fileInfo_.IncreaseRequests();
	}

	while( std::getline( s, line ) && line != "\r" )
	{
		std::string value = boost::trim_copy( line.substr( line.find( ':' ) + 1 ) );
	
		if( istarts_with( line, "Connection:" ) ) 
			keepAlive_ = iequals( value, "keep-alive" );
		else if( istarts_with( line, "X-Nick:" ) ) 
			nick_ = value;
		else if(istarts_with(line, "User-Agent:") && client_ != value)
		{
			client_ = value;
			if(System::GetSecurity()->AgentRestricted(client_))
				throw Unhandled(403, "Client software is restricted");
		}
		else if( istarts_with( line, "Range:" ) )
		{
			file_offset_t first = 0;
			file_offset_t last = 0;

			int result = sscanf( value.c_str(), "bytes=%llu-%llu", &first, &last );
			if( result == 0 ) throw Unhandled(416, "Couldn't parse range");
			if( result == 1 ) range_.SetBoundary( first, fileInfo_.Size() - 1);
			if( result == 2 ) range_.SetBoundary( first, last );
		}
	}

	if( range_.Empty() ) throw std::range_error( "Range is empty" );
//	std::cout << range_.Last() << " " << fileInfo.Size() << std::endl;
	if( range_.Last() >= fileInfo_.Size() ) 
		throw Unhandled(416, "Range is too large" );
}

void UploaderImp::HandleRequest()
{
	range_.Reset();
	offset_ = 0;
	keepAlive_ = true;

	std::ostream r( &response_ );

	try
	{
		try
		{ 
			if(status_ == QUEUED && System::Now() - timestamp_ < std::time_t(System::GetSettings()->net.pollMin))
				throw Unhandled(503, "Polling too often");

			TranslateRequest(); 
			if( fileInfo_.Partial() ) 
				throw Unhandled(500, "Partial uploads not implemented");
			file_ = System::GetFileMgr()->Open( fileInfo_.Path(), fio::read );
			path_ = fileInfo_.Path();
		}
		catch(Unhandled& e)
		{
			r << "HTTP/1.1 " << e.Status() << " " << e.what() << "\r\n\r\n";
			throw;
		}
		catch( std::exception& e )
		{
			r << "HTTP/1.1 500 Internal server error\r\n\r\n";
			throw;
		}
	}
	catch(std::exception& e)
	{
		System::LogBas() << "Request error in uploader " << endpoint_ << " " << e.what() << std::endl;
		status_ = CLOSING;
		ba::async_write( *pSock_, response_, boost::bind( &UploaderImp::OnResponseSent, shared_from_this(), ba::placeholders::error ) );
		return;
	}

	if(queuePos_)
	{
		System::LogAdv() << "Queuing uploader " << endpoint_ << ": " << queuePos_ << " of " << queueSize_ << std::endl;
		r << "HTTP/1.1 503 Busy queued\r\n";
		ComposeCommonHeaders(r);
		r << "X-Queue: position=" << queuePos_ 
			<< ",length=" << queueSize_
			<< ",limit=" << System::GetSettings()->net.maxUploads
			<< ",pollMin=" << System::GetSettings()->net.pollMin
			<< ",pollMax=" << System::GetSettings()->net.pollMax
			<< ",id=\"general\""
			<< "\r\n";
		r << "Content-Length: 0\r\n";
		r << "\r\n";
		status_ = QUEUED;
	}
	else
	{
		bool partial = range_.Length() < fileInfo_.Size();
		if( !partial ) r << "HTTP/1.1 200 OK";
		else r << "HTTP/1.1 206 Partial content";
		r << "\r\n";

		ComposeCommonHeaders( r );

		if( partial ) r << "Content-Range: bytes=" << range_.First() << "-" << range_.Last() << "/" << fileInfo_.Size() << "\r\n"; 
		r << "Content-Length: " << (method_ == GET ? range_.Length() : 0) << "\r\n"; 
		r << "\r\n";

		status_ = TRANSFER;
		transferStarted_ = System::Now();
	}

	ba::async_write( *pSock_, response_, boost::bind( &UploaderImp::OnResponseSent, shared_from_this(), ba::placeholders::error ) );
}

void UploaderImp::OnResponseSent( const bs::error_code& err )
{
	if(status_ == CLOSED) return;

	if(!err && status_ != CLOSING && fileInfo_.Lock())
	{
		if(status_ == TRANSFER)
		{
			System::LogAdv() << "Sending range " 
				<< range_.First() << "-" << range_.Last() << " of " << fileInfo_.Name() << " to " << endpoint_ << std::endl;
			Transfer();
		}
		else 
		{
			assert(status_ == QUEUED);
			ba::async_read_until( *pSock_, request_, "\r\n\r\n", boost::bind( &UploaderImp::OnRequest, shared_from_this(), ba::placeholders::error ) );
		}
	}
	else if(err != ba::error::operation_aborted)
	{
		if( status_ != CLOSING )
		{
			if(!fileInfo_.Lock()) 
				System::LogDev() << "Terminating upload to " << endpoint_ << " => File is no longer available" << std::endl;
			else System::LogDev() << "Uploader::OnResponseSent " << endpoint_ << " error => " << err.message() << std::endl;
		}
		DetachMe();
	}
}

void UploaderImp::Transfer()
{
	timestamp_ = System::Now();

	const file_offset_t want = std::min(range_.Length() - offset_, file_offset_t(BULK_SIZE)); 
	const uint reserved = System::GetTraffOut()->Use(endpoint_, want);

	if(reserved)
	{
		bwlimit_ = false;
		buffer_.resize(reserved);
		try{ file_.Read( &buffer_[0], buffer_.size(), range_.Offset() + offset_ ); }
		catch(AsyncFile::Error& err)
		{
			System::LogBas() << "File reading error in uploader: " << err.what() << std::endl;
			throw;
		}

		offset_ += buffer_.size();
		totalSent_ += buffer_.size();

		ba::async_write( *pSock_, ba::buffer( buffer_ ), boost::bind( &UploaderImp::OnTransfer, shared_from_this(), ba::placeholders::error ) );
	}
	else bwlimit_ = true;
}

void UploaderImp::OnTransfer( const bs::error_code& err )
{
	if(status_ == CLOSED) return;

	if(!err && fileInfo_.Lock())
	{
		timestamp_ = System::Now();
		assert(offset_ <= range_.Length());
		if( offset_ == range_.Length() ) 
		{
			status_ = IDLE;
			System::LogAdv() << "Range " << range_.First() << "-" << range_.Last() 
				<< " of " << fileInfo_.Name() << " has been transferred to " << endpoint_ << std::endl;

			if(!keepAlive_)
			{
				System::LogBas() << "Disconnecting uploader " << endpoint_ << " => connection: close" << std::endl;
				DetachMe();
				return;
			}

			ba::async_read_until( *pSock_, request_, "\r\n\r\n", boost::bind( &UploaderImp::OnRequest, shared_from_this(), ba::placeholders::error ) );
		}
		else
		{
			assert( offset_ < range_.Length() );
			try { Transfer(); }
			catch( std::exception& )
			{
				DetachMe();
				return;
			}
		}
	}
	else if( err != ba::error::operation_aborted )
	{
		if(!fileInfo_.Lock()) 
			System::LogDev() << "Terminating upload to " << endpoint_ << " => File is no longer available" << std::endl;
		else System::LogBas() << "Uploader::OnTransfer " << endpoint_ << " error => " << err.message() << std::endl;
		DetachMe();
	}
}

void UploaderImp::ComposeCommonHeaders(std::ostream& r)
{
	r << "Host: " << endpoint_ << "\r\n";
	if( keepAlive_ )
		r << "Connection: Keep-Alive" << "\r\n";
	r << "X-Features: g2/1.0" << "\r\n";
	r << "User-Agent: " << Ident::fullname << "\r\n";
	r << "Listen-IP: " << System::SelfEndpoint() << "\r\n";
	r << "X-Nick: " << System::GetSettings()->general.userName << "\r\n";
}

void UploaderImp::OnTimer( std::time_t now )
{
	try
	{
		const std::time_t timeout = (status_ == QUEUED ? System::GetSettings()->net.pollMax : TRANSFER_TIMEOUT);

		if( now - timestamp_ > timeout )
		{
			System::LogBas() 
				<< "Uploader " << endpoint_ << " stalled (" << timeout << "). "  
				<< "Status: " << (status_ == QUEUED ? "queued" : "transfer") << ". "
				<< "Range: " << range_.First() << "-" << range_.Last() << ". " << std::endl;;
			Close();
			DetachMe();
			return;
		}

		if(bwlimit_ && status_ == TRANSFER) Transfer();
	}
	catch(std::exception& err)
	{
		System::LogBas() << "Exception in uploader " << endpoint_ << " => " << err.what() << std::endl;
		Close();
		DetachMe();
	}
}

void UploaderImp::DetachMe()
{
	Close();
	Ptr me = shared_from_this();
	System::GetUploadMgr()->Detach( me );
}

void UploaderImp::Close()
{
	if(status_ != CLOSED)
	{
		pSock_->close();
		if(!file_.IsClosed()) file_.Close();
		status_ = CLOSED;
	}
}


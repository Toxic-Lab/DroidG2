/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009  Andrey Stroganov <savthe@gmail.com>

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

#include "downloader_impl.hpp"
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "identity.hpp"
#include <assert.h>
#include "download.hpp"
#include "system.hpp"
#include "settings.hpp"
#include "filemap.hpp"
#include "tigertree.hpp"
#include "trafmanager.hpp"
#include "android/log.h"

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::istarts_with;
using boost::trim;

uint DownloaderImpl::instanceCount_ = 0;

namespace {

template <typename T>
void ExtractAvailableRanges(std::string value, T out)
{
	if(!boost::istarts_with(value, "bytes")) return;
	value.erase(0, 7);
	boost::replace_all(value, ",", " ");
	boost::replace_all(value, "-", " ");

	std::stringstream s(value);
	file_offset_t first, last;
	while((s >> first >> last) && first <= last)
	{
		FileRange r;
		r.SetBoundary(first, last);
		*out++ = r;
	}
}

void ExtractAltLocation(const std::string& value, Download& download)
{	
	std::stringstream s(value);
	std::string line;
	while(std::getline(s, line, ','))
	{
		trim(line);
		const std::string uri = line.substr(0, line.find(' '));
		SourceHost host;
		if(host.FromUri(uri))
		       	download.AddSource(host);
	}
}

}


DownloaderImpl::DownloaderImpl(Download& dl, SourceHostEx::Ptr pHost):
	pSocket_(new ba::ip::tcp::socket(System::GetIo())),
	download_(dl),
	response_(BUFFER_SIZE),
	pushed_(false),
	received_(0),
	status_(CONNECT),
	pHost_(pHost),
	tasksCount_(0)
{
	++instanceCount_;

	pHost_->connections++;
	timestamp_ = System::Now();
}

DownloaderImpl::DownloaderImpl(Download& dl, TcpSocketPtr pSocket, SourceHostEx::Ptr pHost):
	pSocket_(pSocket),
	download_(dl),
	response_(BUFFER_SIZE),
	pushed_(true),
	received_(0),
	status_(CONNECT),
	pHost_(pHost),
	tasksCount_(0)
{
	++instanceCount_;

	pHost_->connections++;
	timestamp_ = System::Now();
}

DownloaderImpl::~DownloaderImpl()
{
	assert(status_ == CLOSED);
	--instanceCount_;
}

void DownloaderImpl::Connect()
{
	ba::ip::tcp::endpoint asioEndpoint(ba::ip::address_v4(pHost_->endpoint.address.Reversed()), pHost_->endpoint.port);
	pSocket_->async_connect(asioEndpoint, boost::bind(&DownloaderImpl::OnConnected, shared_from_this(), ba::placeholders::error));
}

void DownloaderImpl::CancelTask()
{
	if(task_ == FETCH_FILE && pFileMap_ && !myRange_.Empty())
	{
		pFileMap_->SetEmpty(myRange_);
		myRange_.Reset();
	}
}

/*
void DownloaderImpl::Push()
{
	const GUID guid = pHost_->guid;
	if(!guid.Empty() && !pHost_->hubs.empty())
		download_.Push(guid, pHost_->hubs[rand() % pHost_->hubs.size()]);
}
*/

void DownloaderImpl::OnTimer()
{
	if( (status_ == CHAT || status_ == CONNECT || status_ == TRANSFER) && System::Since(timestamp_) > 10 ) 
	{
		if(status_ == CONNECT)
			System::LogAdv() << "Download connection timeout: " << pHost_->endpoint << std::endl;
		else 
			System::LogAdv() << "Download connection stalled: " << pHost_->endpoint << std::endl;
		DetachMe();
		return;
	}

	if( status_ == TRANSFER )
	{
		if(!active_) Transfer();
	}
	else if( status_ == QUEUE )
	{
		if(System::Since(queue_.timestamp) > queue_.pollMin + 3)
		{
			status_ = IDLE;
			download_.OnIdle( shared_from_this() );
		}
	}
}

void DownloaderImpl::OnConnected(const bs::error_code& err)
{
	if(status_ == CLOSED) return;

	if(!err)
	{
		timestamp_ = System::Now();
		status_ = IDLE;
		download_.OnIdle(shared_from_this());
	}
	else if(err != ba::error::operation_aborted)
	{
		status_ = ERROR;
		System::LogAdv() << "Error connecting to " << pHost_->endpoint << " " << err.message() << std::endl;
		DetachMe();
	}
}

void DownloaderImpl::DetachMe()
{
	Close();
	boost::shared_ptr <DownloaderImpl> me(shared_from_this());
	download_.Detach(me);
}

void DownloaderImpl::Transfer()
{
	const file_offset_t want = std::min(myRange_.Length() - offset_, file_offset_t(16 * 1024));
	const uint reserved = System::GetTraffIn()->Use(pHost_->endpoint, want);
	if(reserved)
	{
		active_ = true;
		buffer_.resize(reserved);
		ba::async_read(*pSocket_, ba::buffer(buffer_), ba::transfer_all(), 
				boost::bind(&DownloaderImpl::OnTransfer, shared_from_this(), ba::placeholders::error));
	}
	else active_ = false;
}

void DownloaderImpl::TaskComplete()
{
	++tasksCount_;
	if(task_ == FETCH_FILE)
	{
		pFileMap_->SetFull(myRange_);
	}
	else if(task_ == FETCH_TIGER)
	{
		Hashes::TigerTree t;
		if(t.FromBytes(tigerBuffer_, download_.GetFileInfo().Size()))
			download_.OnTiger(t);
		else
			System::LogAdv() << "Can't compose tiger tree" << std::endl;
	}
	else assert(0);

	myRange_.Reset();
	status_ = IDLE;
	download_.OnIdle(shared_from_this());
}

void DownloaderImpl::OnData()
{
	timestamp_ = System::Now();
	received_ += buffer_.size();

	if(task_ == FETCH_FILE)
		file_.Write(&buffer_[0], buffer_.size(), myRange_.Offset() + offset_);
	else if(task_ == FETCH_TIGER)
		tigerBuffer_.insert(tigerBuffer_.end(), buffer_.begin(), buffer_.end());
	else assert(0);

	offset_ += buffer_.size();
	//if(task_ == FETCH_TIGER) std::cout << offset_ << "/" << myRange_.Length() << std::endl;
	if(offset_ == myRange_.Length())
		TaskComplete();
	else 
	{
		assert(offset_ < myRange_.Length());
		Transfer();
	}
}

uint DownloaderImpl::AvrRate() const 
{
	if(status_ == TRANSFER)
	{
		std::time_t duration = System::Now() - transferStart_;
		return duration == 0 ? offset_ : offset_ / duration;
	}
	return 0;
}


void DownloaderImpl::OnTransfer(const bs::error_code& err)
{
	if(status_ == CLOSED) return;

	if(!err)
	{
		//assert(socket_.is_open());
		active_ = false;
		OnData();
	}
	else if(err != ba::error::operation_aborted)
	{
		status_ = ERROR;
		System::LogAdv() << "Downloader::OnTransfer " << pHost_->endpoint << " error => " << err.message() << std::endl;;
		DetachMe();
	}
}

void DownloaderImpl::Close()
{
	if(status_ == CLOSED) return;
	status_ = CLOSED;
	CancelTask();
	pHost_->connections--;
	file_.Close();
	pSocket_->close();
}

void DownloaderImpl::OnHeadersSent(const bs::error_code& err)
{
	if(status_ == CLOSED) return;

	if(!err) 
	{
		timestamp_ = System::Now();
		ba::async_read_until(*pSocket_, response_, "\r\n\r\n", 
				boost::bind(&DownloaderImpl::OnHeadersReceived, shared_from_this(), ba::placeholders::error));
	}
	else if(err != ba::error::operation_aborted)
	{
		status_ = ERROR;
		System::LogAdv() << "DownloaderImpl::OnHeadersSent " << pHost_->endpoint << " error => " << err.message() << std::endl;
		DetachMe();
	}
}

		
void DownloaderImpl::HandleResponse()
{
	std::istream s(&response_);
	int code;
	offset_ = 0;

	char debug[200];

	std::string header;
	if( !(s >> header >> code) )
		throw std::runtime_error("Can't parse reply code");
	getline(s, header);

	file_offset_t first = 0, last = 0, total = 0, contentLength = 0;

	while(std::getline(s, header) && header != "\r")
	{
		std::string value = boost::trim_copy(header.substr(header.find(':') + 1));

		if(istarts_with(header, "X-Available-Ranges:"))
		{
			ranges_.clear();
			ExtractAvailableRanges(value, std::back_inserter(ranges_));
		}
		else if(istarts_with(header, "Alt-Location:"))
		{
			ExtractAltLocation(value, download_);
		}
		else if(tigerUri_.empty() && istarts_with(header, "X-Thex-URI:"))
 		{
 			//urn:tree:tiger/:BASE32HASH
 			std::string::size_type i = value.find("urn:tree:tiger/:");
			if(i != std::string::npos)
			{
				tigerUri_ = "/gnutella/tigertree/v3?";
				while(i < value.size() && value[i] != '&' && value[i] != ';')
					tigerUri_.push_back(value[i++]);
			}
		}
		else if(istarts_with(header, "X-PerHost:"))
		{
			uint n = lexical_cast<uint>(value);
			if( n < 4 ) pHost_->maxConnections = n;
		}
		else if(istarts_with(header, "Retry-After:"))
		{
			//pHost_->connectAfter = System::Now() + lexical_cast<uint>(value);
			//handled in downloader
		}
		else if(istarts_with(header, "Content-Length:"))
			contentLength = lexical_cast<file_offset_t>(value);
		else if(istarts_with(header, "X-Queue:"))
		{
			if(code == 503)
			{
				CancelTask();
				status_ = QUEUE;
				UpdateQueue(value);
			}
		}
		else if(istarts_with(header, "Content-Range:"))
		{
			sprintf(debug, "set content range of host %s %s", pHost_->endpoint.ToString().c_str(), value.c_str());
			__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
			if(sscanf(value.c_str(), "bytes%*c%u-%u/%u", &first, &last, &total) < 3 || first > last || total == 0)
				throw std::range_error("Host provided invalid range info");
		}
		else
		{
		}
	}

	switch (code)
	{
		case 200: case 206:
		{
			if(task_ == FETCH_FILE)
			{
				if(myRange_.First() != first || myRange_.Last() != last)
				{
					System::LogAdv() << "Host " << pHost_->endpoint << " is forcing range " << first << "-" << last <<
						" instead of " << myRange_.First() << "-" << myRange_.Last() << std::endl;

					sprintf(debug, "check content range of host not valid %s ", pHost_->endpoint.ToString().c_str());
					__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);

					throw std::range_error("Forced ranges are not allowed");
				}
				//std::cout << "Getting range " << myRange_.First() << "-" << myRange_.Last() << 
				//	" from " << pHost_->endpoint << std::endl;
			}
			else
			{
				assert(task_ == FETCH_TIGER);
				if(contentLength == 0) throw std::range_error("Length of tiger tree is 0");
				if(contentLength >= 24 * 1024) throw std::range_error("Tiger tree is too large");
				myRange_.Set(0, contentLength);
				System::LogAdv() << "Getting tiger tree from " << pHost_->endpoint << std::endl;

			}
			status_ = TRANSFER;
			offset_ = 0;
			timestamp_ = System::Now();
			transferStart_ = System::Now();
			buffer_.clear();

			if(response_.size() > 0)
			{
				uint n = response_.size();
				buffer_.reserve(n);
				while(n --> 0 ) buffer_.push_back(s.get());
				OnData();
			}
			else Transfer();

			break;
		}
		case 503:
 		{
			System::LogAdv() << "Host " << pHost_->endpoint << " busy to upload.";
			if(status_ == QUEUE) System::Log() << " Waiting in queue " << queue_.pos << " of " << queue_.total;
			System::Log() << std::endl;
			if(status_ != QUEUE) throw std::runtime_error("Refused");
			break;
		}
		default: throw std::runtime_error("Refused");
	}
}

void DownloaderImpl::UpdateQueue(const std::string& info)
{
	int pollMin = 0;
	int pollMax = 0;
	queue_.timestamp = System::Now();
	std::vector< std::string > v;
	boost::split(v, info, boost::is_any_of(","));
	for(uint i = 0; i < v.size(); ++i)
	{
		if(v[i].empty()) continue;
		std::sscanf(v[i].c_str(), "position=%d", &queue_.pos);
		std::sscanf(v[i].c_str(), "length=%d", &queue_.total);
		std::sscanf(v[i].c_str(), "pollMin=%d", &pollMin);
		std::sscanf(v[i].c_str(), "pollMax=%d", &pollMax);
		queue_.pollMin = pollMin;
		queue_.pollMax = pollMax;
	}
}

void DownloaderImpl::OnHeadersReceived(const bs::error_code& err)
{
	if(status_ == CLOSED) return;

	if(!err)
	{
		try { HandleResponse(); }
		catch (std::exception& e)
		{
			System::LogAdv() << "Error in downloader " << pHost_->endpoint << " => " << e.what() << std::endl; 
			status_ = ERROR;
			DetachMe();
		}
	}
	else if (err != ba::error::operation_aborted)
	{
		status_ = ERROR;
		System::LogDev() << "Downloader::OnHeadersReceived " << pHost_->endpoint << " error => " << err.message() << std::endl;
		DetachMe();
	}
}

void DownloaderImpl::ComposeCommonHeaders(std::ostream& ostr)
{
	ostr << "Host: " << pHost_->endpoint.ToString() << "\r\n";
	ostr << "Connection: Keep-Alive" << "\r\n";
	ostr << "X-Features: g2/1.0" << "\r\n";
	ostr << "User-Agent: " << Ident::fullname << "\r\n";
	ostr << "Listen-IP: " << System::SelfEndpoint() << "\r\n";
	ostr << "X-Queue: 0.1" << "\r\n";
	ostr << "X-Nick: " << System::GetSettings()->general.userName << "\r\n";
}

void DownloaderImpl::FetchTiger()
{
	task_ = FETCH_TIGER;
	tigerBuffer_.clear();
	assert(HasTiger());
	timestamp_ = System::Now();
	System::LogAdv() << "Requesting tiger tree from " << pHost_->endpoint << std::endl;
	std::ostream r(&request_);
	r << "GET " << tigerUri_ << " HTTP/1.1" << "\r\n";
	r << "Accept: application/tigertree-breadthfirst" << "\r\n";
	r << "Range: bytes=0-" << "\r\n";
	ComposeCommonHeaders(r);
	r << "\r\n";
	ba::async_write(*pSocket_, request_, boost::bind(&DownloaderImpl::OnHeadersSent, shared_from_this(), ba::placeholders::error));
	status_ = CHAT;
}

void DownloaderImpl::FetchFile(std::string uri, SharedFile file, FileMapPtr p)
{
	task_ = FETCH_FILE;
	file_ = file;
	pFileMap_ = p;
	myRange_ = pFileMap_->GetEmptyRange(ranges_.begin(), ranges_.end());
	timestamp_ = System::Now();
//	std::cout << "Requesting: " << uri << std::endl;
	if(!pHost_->uri.empty()) uri = pHost_->uri;
	if(myRange_.Empty())
	{
		System::LogAdv() << "Downloader " << pHost_->endpoint << " has no work." << std::endl;
		DetachMe();
	}
	else
	{
		//verb.bas << "Requesting from " << pHost_->endpoint << " file range " << myRange_.First() << "-" << myRange_.Last() << verb.flush;
		pFileMap_->SetActive(myRange_);
		std::ostream r(&request_);
		r << "GET " << uri << " HTTP/1.1" << "\r\n";
		r << "Range: bytes=" << myRange_.First() << "-" << myRange_.Last() << "\r\n";
		ComposeCommonHeaders(r);
		r << "\r\n";

		ba::async_write(*pSocket_, request_, boost::bind(&DownloaderImpl::OnHeadersSent, shared_from_this(), ba::placeholders::error));
		status_ = CHAT;
	}
}


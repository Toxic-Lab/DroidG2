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

#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "webuiclient.hpp"
#include "webuiserver.hpp"
#include "settings.hpp"
#include "ui/command.hpp"
#include "webuicontrol.hpp"
#include "ui/template.hpp"
#include "filesystem.hpp"
#include "asyncfile.hpp"
#include "system.hpp"
#include "url.hpp"
#include "contenttype.hpp"

using boost::iequals;
using boost::starts_with;
using FileSystem::Exists;
using FileSystem::FileSize;
using FileSystem::Extension;

typedef boost::shared_ptr <AsyncFile> AsyncFilePtr;

namespace {

const unsigned int CONTENT_MAX_SIZE = 32 * 1024 * 1024;
const std::time_t ALIVE_TIME = 180;

}

WebUIClient::WebUIClient():
	socket_(System::GetIo()), 
	message_(64 * 1024),
	closed_(false)
{
}

Ip::Endpoint WebUIClient::GetEndpoint()
{
	Ip::Endpoint e(socket_.remote_endpoint().address().to_v4().to_ulong(), socket_.remote_endpoint().port());
	e.address.Reverse();
	return e;
}

void WebUIClient::Start()
{
	ba::async_read_until(socket_, message_, "\r\n\r\n", boost::bind(&WebUIClient::OnHeadersReceived, shared_from_this(), ba::placeholders::error));
}

void WebUIClient::OnHeadersReceived(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		try { TranslateHeaders(); }
		catch(std::exception& e)
		{
			System::LogBas() << "Webui client " << GetEndpoint() << " couldn't handle request => " << e.what() << std::endl;
			DetachMe();
			return;
		}
	}
	else if(err != ba::error::operation_aborted)
	{
		System::LogBas() << "Webui client " << GetEndpoint() << " => " << err.message() << std::endl;
		DetachMe();
		return;
	}
}

void WebUIClient::TranslateHeaders()
{
	fileRange_.Reset();
	keepAlive_ = 0;
	unsigned int length = 0;
	content_.clear();

	std::string method, proto;
	std::istream r(&message_);

	if(!(r >> method >> object_ >> proto)) 
		throw std::runtime_error("Couldn't parse first line"); 
	if(iequals(method, "GET")) method_ = GET;
	else if(iequals(method, "POST")) method_ = POST;
	else throw std::runtime_error("Method is not supported => " + method);

	std::string line;
	r.ignore(1);
	while(getline(r, line) && line != "\r")
	{
		std::stringstream s(line);
		std::string header;
		s >> header;

		if(iequals(header, "Connection:"))
		{
			std::string value;
			s >> value;
			if(iequals(value, "keep-alive"))
			keepAlive_ = System::Now() + ALIVE_TIME + 1;
		}
		else if(iequals(header, "Content-length:"))
		{
			s >> length;
		}
		else if(iequals(header, "Range:" ))
		{
			file_offset_t first = 0;
			file_offset_t last = 0;
			std::string value;
			s >> value;

			const int result = sscanf( value.c_str(), "bytes=%llu-%llu", &first, &last );
			if( result == 0 ) throw std::runtime_error( "Can't parse range" );
			if( result == 1 ) 
			{
				fileRange_.Offset(first);
				fileRange_.Length(0);
			}
			if( result == 2 ) fileRange_.SetBoundary( first, last );
		}
	}

	if(length > CONTENT_MAX_SIZE || message_.size() > length)
		throw std::runtime_error("Content length is too large");

	if(length)
	{
		while(message_.size() > 0)
			content_.push_back(r.get());
		length -= content_.size();
	}

	if(length)
	{
		buffer_.resize(length);
		ba::async_read(socket_, ba::buffer(buffer_), ba::transfer_all(), boost::bind(&WebUIClient::OnContent, this, ba::placeholders::error));
	}
	else HandleRequest();
}

void WebUIClient::Close()
{
	if(!closed_)
	{
		if(file_.Opened()) file_.Close();
		socket_.close();
		closed_ = true;
	}
}

void WebUIClient::DetachMe()
{
	boost::shared_ptr<WebUIClient> me(shared_from_this());
	Close();
	System::GetWebUi()->Detach(me);
}

void WebUIClient::OnContent(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		content_.insert(content_.end(), buffer_.begin(), buffer_.end());
		HandleRequest();
	}
	else if(err != ba::error::operation_aborted)
		DetachMe();
}

void WebUIClient::HandleRequest()
{
	System::LogAdv() << "WebUI request: " << object_ << std::endl;

	try
	{
		if(method_ == GET && starts_with(object_, "/img"))
		{
			object_.insert(0, System::GetWebUi()->GetStyleRoot());
			ServeFile();
		}
		else if(method_ == GET && starts_with(object_, "/fetch"))
		{
			object_ = System::GetSettings()->general.complete + Url::Decode(object_.substr(6));
			ServeFile();
		}
		else DoCommand();

	}
	catch(std::exception& e)
	{
		System::LogBas() << "WebUI error: " << e.what() << std::endl;
		std::ostream r(&message_);
		r << "503 Internal Error\r\n\r\n";
		ba::async_write(socket_, message_, boost::bind(&WebUIClient::OnFinal, this, ba::placeholders::error));
	}
}

void WebUIClient::ServeFile()
{
	std::ostream r(&message_);

	const bool notFound = 
		object_.empty() ||
		object_.find("..") != std::string::npos ||
		!Exists(object_);

	if(notFound)
	{
		System::LogBas() << "Webui requested object has not been found: " << object_ << std::endl;
		r << "404 Not Found\r\n";
		ba::async_write(socket_, message_, boost::bind(&WebUIClient::OnFinal, this, ba::placeholders::error));
		return;
	}

	file_.Open(object_, fio::read);
	if(!file_.Opened())
		throw std::runtime_error("Couldn't open file " + object_);

	const file_offset_t fileSize = FileSize(object_);

	if(fileRange_.Length() == 0) 
		fileRange_.Length(fileSize - fileRange_.Offset());

	const bool partial = fileRange_.Length() != fileSize;

	if(partial) r << "HTTP/1.1 206 Partial content\r\n";
	else r << "HTTP/1.1 200 OK\r\n";
	r << "Content-Type: " << ContentType::Resolve(Extension(object_)) << "\r\n";
	r << "Content-Length: " << fileRange_.Length() << "\r\n";
	if(keepAlive_) r << "Keep-Alive: " << ALIVE_TIME << "\r\n";
	r << "\r\n";
	
	ba::async_write(socket_, message_, boost::bind(&WebUIClient::OnFileTransfer, shared_from_this(), ba::placeholders::error));
}

void WebUIClient::OnFileTransfer(const bs::error_code& err)
{
	if(closed_) return;
	if(err == ba::error::operation_aborted) return;
	if(err)
	{
		System::LogBas() << "Webui error during file upload => " << err.message() << std::endl;
		DetachMe();
		return;
	}

	const file_offset_t length = std::min(fileRange_.Length(), file_offset_t(16 * 1024));
	if(length == 0)
	{
		OnTransferCompleted(err);
		return;
	}

	buffer_.resize(length);
	file_.Read(&buffer_[0], buffer_.size(), fileRange_.Offset());
	ba::async_write(socket_, ba::buffer(buffer_), boost::bind( &WebUIClient::OnFileTransfer, shared_from_this(), ba::placeholders::error));
	fileRange_.Offset(fileRange_.Offset() + length);
	fileRange_.Length(fileRange_.Length() - length);
}

void WebUIClient::DoCommand()
{
	if(content_.size() > 0)
	{
		object_.push_back('&');
		object_.insert(object_.end(), content_.begin(), content_.end());
	}

	if(!object_.empty()) object_.erase(0, 1);

	if(object_.empty()) object_ = StaticName::ToString(Ui::Tags::home);

	Ui::Command cmd;
	
	std::string::size_type cmdSepPos = object_.find('?');
	if(cmdSepPos != std::string::npos)
		object_[cmdSepPos] = '&';

	cmd.FromString(object_, '&');

	const std::string root = System::GetWebUi()->GetStyleRoot();
	if(root.empty()) throw std::runtime_error("WARNING: Couldn't find webui style folder. Please set valid style name in sharelin.conf");

	std::string result;
	WebuiControl cnt(cmd, result);
	content_.clear();
	content_.reserve(result.size());
	std::copy(result.begin(), result.end(), std::back_inserter(content_));

	std::ostream r(&message_);
	r << "HTTP/1.1 200 OK\r\n";
	r << "Content-Type: text/html\r\n";
	r << "Content-Length: " << content_.size() << "\r\n";
	if(keepAlive_) 
		r << "Keep-Alive: " << ALIVE_TIME << "\r\n";
	r << "\r\n";

	ba::async_write(socket_, message_, boost::bind(&WebUIClient::OnPageHeaderSent, this, ba::placeholders::error));
}

void WebUIClient::OnFinal(const bs::error_code& err)
{
	if(closed_) return;

	if(err != ba::error::operation_aborted)
		DetachMe();
}

void WebUIClient::OnPageHeaderSent(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		if(content_.empty()) OnTransferCompleted(err);
		else ba::async_write(socket_, ba::buffer(content_), boost::bind(&WebUIClient::OnTransferCompleted, this, ba::placeholders::error));
	}
	else if(err != ba::error::operation_aborted)
		DetachMe();
}

void WebUIClient::OnTransferCompleted(const bs::error_code& err)
{
	if(closed_) return;

	if(!err)
	{
		if(System::Now() > keepAlive_)
		{
			DetachMe();
			return;
		}

		ba::async_read_until(socket_, message_, "\r\n\r\n", boost::bind(&WebUIClient::OnHeadersReceived, shared_from_this(), ba::placeholders::error));
	}
	else if(err != ba::error::operation_aborted)
		DetachMe();
}


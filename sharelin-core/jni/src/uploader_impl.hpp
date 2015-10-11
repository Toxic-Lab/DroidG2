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

#ifndef UPLOADER_IMP_HPP
#define UPLOADER_IMP_HPP

#include "io.hpp"
#include "types.hpp"
#include "filerange.hpp"
#include "filemap.hpp"
#include "sharedfile.hpp"
#include <vector>
#include "uploader.hpp"
#include <boost/enable_shared_from_this.hpp>

class Gatekeeper;

class UploaderImp: public Uploader, public boost::enable_shared_from_this<UploaderImp>
{
public:
	UploaderImp(const Gatekeeper*);
	~UploaderImp();
	Ip::Endpoint Address() const { return endpoint_; }
	void OnTimer( std::time_t );
	void SetQueue(unsigned pos, unsigned size ) { queuePos_ = pos; queueSize_ = size; }
	void Close();
	std::string Nick() const { return nick_; }
	Share::ManagedInfo GetFileInfo() const { return  fileInfo_; }
	file_offset_t Sent() const { return totalSent_; }
	std::string Path() const { return path_; }
	uint AvrRate() const;
	StatusType Status() const { return status_; }
	void Start();
	std::string ClientName() const { return client_; }

private:
	enum MethodType { GET, HEAD };

	class Unhandled: public std::runtime_error
	{
	public:
		Unhandled(int status, const std::string& msg): std::runtime_error(msg), status_(status) { }
		int Status() const { return status_; }
	private:
		int status_;
	}; 

	void DetachMe();
	void OnRequest( const bs::error_code& );
	void HandleRequest();
	void TranslateRequest();
	void OnResponseSent( const bs::error_code& );
	void ComposeCommonHeaders( std::ostream& );
	void Transfer();
	void OnTransfer( const bs::error_code& );

	TcpSocketPtr pSock_;
	Ip::Endpoint endpoint_;
	std::time_t timestamp_;
	FileRange range_;
	SharedFile file_;
	boost::shared_ptr< FileMap > pMap_;
	file_offset_t totalSent_;
	file_offset_t offset_;
	bool bwlimit_;
	bool keepAlive_;
	ba::streambuf request_;
	ba::streambuf response_;
	std::vector< char > buffer_;
	MethodType method_;
	StatusType status_;
	std::string urn_;
	Share::ManagedInfo fileInfo_;
	std::string nick_;
	std::time_t transferStarted_;
	uint queuePos_;
	uint queueSize_;
	std::time_t queued_;
	std::string path_;
	std::string client_;
};

#endif //UPLOADER_IMP_HPP


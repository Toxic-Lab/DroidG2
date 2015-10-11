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

#ifndef DOWNLOADER_IMPL_HPP
#define DOWNLOADER_IMPL_HPP

#include <boost/enable_shared_from_this.hpp>
#include "types.hpp"
#include "ip.hpp"
#include "io.hpp"
#include <vector>
#include <ostream>
#include <string>
#include "filemapptr.hpp"
#include "filerange.hpp"
#include "sourcehost.hpp"
#include "downloader.hpp"

class TrafManager;

class DownloaderImpl: public Downloader, public boost::enable_shared_from_this<DownloaderImpl>
{
public:
	DownloaderImpl(Download& dl, SourceHostEx::Ptr);	
	DownloaderImpl(Download& dl, TcpSocketPtr, SourceHostEx::Ptr);	
	~DownloaderImpl();

	uint AvrRate() const;
	bool Pushed() const { return pushed_; }
	TaskType Task() const { return task_; }
	uint QueuePos() const { return queue_.pos; }
	StatusType Status() const { return status_; }
	FileRange GetRange() const { return myRange_; }
       	uint QueueTotal() const { return queue_.total; }	
    bool HasTiger() const { return !tigerUri_.empty(); }
    //bool HasTiger() const { return false; }
	file_offset_t Received() const { return received_; }
	SourceHostEx::Ptr HostInfo() const { return pHost_; }
	uint CompletedTasks() const { return tasksCount_; }
	void Connect();

	void Close();
	void OnTimer();
	void FetchTiger();
	void FetchFile(std::string, SharedFile, FileMapPtr);

	static uint Count() { return instanceCount_; }
	
private:
	struct Queue
	{
		Queue(): pollMin(0), pollMax(0), pos(0), total(0), timestamp(0) {}
		std::time_t pollMin;
		std::time_t pollMax;
		std::string name;
		uint pos;
		uint total;
		std::time_t timestamp;
	};

	typedef std::vector<char> BytesContainer;
	typedef FileRange DataRange;
	enum {CONNECT_EXPIRED = 10};
	enum {BUFFER_SIZE = 10*1024};

	void Push();
	void OnData();
	void DetachMe();
	void Transfer();
	void CancelTask();
	void TaskComplete();
	void HandleResponse();
	void UpdateQueue(const std::string&);
	void OnTransfer(const bs::error_code&);
	void OnConnected(const bs::error_code&);
	void ComposeCommonHeaders(std::ostream&);
	void OnHeadersSent(const bs::error_code&);
	void OnHeadersReceived(const bs::error_code&);

	TcpSocketPtr pSocket_;
	Download& download_;
	ba::streambuf response_;
	ba::streambuf request_;
	SharedFile file_;
	FileMapPtr pFileMap_;
	BytesContainer buffer_;
	std::time_t timestamp_;
	std::vector<DataRange> ranges_;
	std::vector<char> tigerBuffer_;
	DataRange myRange_;
	std::string tigerUri_;
	bool active_;
	std::time_t transferStart_;
	file_offset_t offset_;
	Queue queue_;
	bool pushed_;
	file_offset_t received_;
	StatusType status_;
	SourceHostEx::Ptr pHost_;
	TaskType task_;
	uint tasksCount_;
	
	static uint instanceCount_;
};

#endif //DOWNLOADER_IMPL_HPP


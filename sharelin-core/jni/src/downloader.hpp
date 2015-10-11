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

#ifndef DOWNLOADER_HPP
#define DOWNLOADER_HPP

#include <boost/enable_shared_from_this.hpp>
#include "types.hpp"
#include <string>
#include "filemapptr.hpp"
#include "filerange.hpp"
#include "sourcehost.hpp"
#include "sharedfile.hpp"

class Download;

class Downloader
{
public:
	enum TaskType {NONE, FETCH_TIGER, FETCH_FILE};
	enum StatusType {IDLE, ERROR, CONNECT, TRANSFER, CHAT, QUEUE, CLOSED};

	virtual ~Downloader() = 0;
	virtual void Connect() = 0;
	virtual void Close() = 0;
	virtual void OnTimer() = 0;
	virtual void FetchTiger() = 0;
	virtual void FetchFile(std::string, SharedFile, FileMapPtr) = 0;

	virtual uint AvrRate() const = 0;
	virtual bool Pushed() const = 0;
	virtual TaskType Task() const = 0;
	virtual uint QueuePos() const = 0;
	virtual StatusType Status() const = 0;
	virtual FileRange GetRange() const = 0;
       	virtual uint QueueTotal() const = 0;	
	virtual bool HasTiger() const = 0;
	virtual file_offset_t Received() const = 0;
	virtual SourceHostEx::Ptr HostInfo() const = 0;
	virtual uint CompletedTasks() const = 0;
};

#endif //DOWNLOADER_HPP


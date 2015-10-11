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

#ifndef UPLOADER_HPP
#define UPLOADER_HPP

#include "ip.hpp"
#include <ctime>
#include <boost/shared_ptr.hpp>
#include <string>
#include "managedinfo.hpp"
#include "types.hpp"

class Uploader
{
public:
	enum StatusType{IDLE, QUEUED, TRANSFER, CLOSING, CLOSED};
	typedef boost::shared_ptr< Uploader > Ptr;

	virtual Ip::Endpoint Address() const = 0;
	virtual void OnTimer(std::time_t) = 0;
	virtual void SetQueue(unsigned, unsigned) = 0;
	virtual void Close() = 0;
	virtual std::string Nick() const = 0; 
	virtual Share::ManagedInfo GetFileInfo() const = 0;
	virtual file_offset_t Sent() const = 0;
	virtual uint AvrRate() const = 0;
	virtual StatusType Status() const = 0;
	virtual std::string Path() const = 0;
	virtual std::string ClientName() const = 0;
	virtual void Start() = 0;

	virtual ~Uploader() {}
};

#endif //UPLOADER_HPP


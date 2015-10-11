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

#ifndef UPLOADMANAGER_HPP
#define UPLOADMANAGER_HPP

#include <ctime>
#include "io.hpp"
#include "ip.hpp"
#include "uploader.hpp"
#include <vector>

class Gatekeeper;

class UploadManager
{
public:
	UploadManager();
	void OnTimer(std::time_t);
	void Accept(TcpSocketPtr, Ip::Endpoint);
	void Detach(Uploader::Ptr);
	void Accept(const Gatekeeper*);
	void Shutdown();
	unsigned Count() const { return uploaders_.size(); }

	template <typename T>
	void Dump(T out)
	{
		for(unsigned i = 0; i < uploaders_.size(); ++i)
			*out++ = uploaders_[i];
	}

private:
	void UpdateQueue();

	std::vector< Uploader::Ptr > uploaders_;
};

#endif //UPLOADMANAGER_HPP


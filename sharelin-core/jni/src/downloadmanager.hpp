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

#ifndef DOWNLOADMANAGER_HPP
#define DOWNLOADMANAGER_HPP

#include <vector>

#include "types.hpp"
#include "idgen.hpp"
#include "netfwd.hpp"

class DownloadRequest;

class DownloadManager
{
public:
	DownloadManager();
	DownloadPtr Create(const DownloadRequest& request);
	void Start();
	void Shutdown();
	void Update();
	void OnTimer();
	void Detach(DownloadPtr);
	bool AllowDownloader();
	unsigned Count() const { return downloads_.size(); }

	DownloadPtr Get(ID);

	template <typename T>
	void Dump(T out)
	{
		std::copy(downloads_.begin(), downloads_.end(), out);
	}

private:
	typedef std::vector<DownloadPtr> DownloadsContainer;

	DownloadsContainer downloads_;
	Misc::IDGenerator idgen_;
};
#endif //DOWNLOADMANAGER_HPP


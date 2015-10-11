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

#ifndef HISTORY_HPP
#define HISTORY_HPP

#include <vector>
#include "historytypes.hpp"

namespace History {

class Keeper
{
public:
	typedef std::vector<Download> DownloadsContainer;
	typedef std::vector<Upload> UploadsContainer;

	void Add(const Download&);
	void Add(const Upload&);

	void ClearDownloads() { downloads_.clear(); }
	void ClearUploads() { uploads_.clear(); }

	const DownloadsContainer& GetDownloads() const { return downloads_; }
	const UploadsContainer& GetUploads() const { return uploads_; }

private:
	DownloadsContainer downloads_;
	UploadsContainer uploads_;
};

} //namespace History

#endif //HISTORY_HPP




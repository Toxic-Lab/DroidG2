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

#ifndef UI_COMPARERS_HPP
#define UI_COMPARERS_HPP

#include <functional>

#include "ui/sortoption.hpp"
#include "netfwd.hpp"
#include "managedinfo.hpp"

namespace Ui {
namespace Comparers {

struct Download: public std::binary_function<DownloadPtr, DownloadPtr, bool>
{
	Download(SortOption opt): opt_(opt) {}
	bool operator()(DownloadPtr, DownloadPtr) const;
	SortOption opt_;
};

struct SharedFiles: public std::binary_function<Share::ManagedInfo, Share::ManagedInfo, bool>
{
	SharedFiles(SortOption opt): opt_(opt) {}
	bool operator()(Share::ManagedInfo, Share::ManagedInfo) const;
	SortOption opt_;
};

} //namespace Comparers
} //namespace Ui

#endif //UI_COMPARERS_HPP


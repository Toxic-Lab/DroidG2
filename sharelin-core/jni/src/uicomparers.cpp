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

#include <stdexcept>
#include <algorithm>

#include "uicomparers.hpp"
#include "download.hpp"
#include "uicommons.hpp"

namespace Ui {
namespace Comparers {

bool Download::operator()(DownloadPtr p1, DownloadPtr p2) const
{
	if(!opt_.Forward()) p1.swap(p2);

	switch(opt_.SortBy())
	{
		case Tags::name: return p1->GetFileInfo().Name() < p2->GetFileInfo().Name();
		case Tags::size: return p1->GetFileInfo().Size() < p2->GetFileInfo().Size();
		case Tags::done: return DownloadProgress(p1) < DownloadProgress(p2);
		case Tags::rate: return p1->AvrRate() < p2->AvrRate();
		case Tags::sources: return p1->CountDownloaders() < p2->CountDownloaders();
		case Tags::priority: return p1->Priority() < p2->Priority();
		default: return p1->GetFileInfo().Name() < p2->GetFileInfo().Name();
	}
}

bool SharedFiles::operator()(Share::ManagedInfo e1, Share::ManagedInfo e2) const
{
	if(!opt_.Forward()) std::swap(e1, e2);

	switch(opt_.SortBy())
	{
		case Tags::name: return e1.Name() < e2.Name();
		case Tags::size: return e1.Size() < e2.Size();
		case Tags::requests: return e1.Requests() < e2.Requests();
		case Tags::hits: return e1.Hits() < e2.Hits();
		default: return e1.Name() < e2.Name();
	}
}

} //namespace Comparers
} //namespace Ui

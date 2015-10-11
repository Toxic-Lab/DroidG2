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

#ifndef WEBUICONTROL_HPP
#define WEBUICONTROL_HPP

#include <string>
#include "types.hpp"
#include "uicommons.hpp"
#include "ui/template.hpp"

namespace Ui
{
	class Command;
	class Prefs;
}

class WebuiControl 
{
public:
	WebuiControl(const Ui::Command& cmd, std::string& result);

private:
	void Do();

	//controls
	void CntHome();
	void CntCreateSearch();
	void CntSearchResults();
	void CntShowAllDownloads();
	void CntShowAllUploads();
	void CntCreateDownload();
	void CntShowSettings();
	void CntShowSearches();
	void CntShowDownload();
	void CntCreateDownloadFromLink();
	void CntCreateDownloadFromEd2k(const std::string&);
	void CntCreateDownloadFromMagnet(const std::string&);
	void CntShowIncoming();
	void CntShowLog();
	void CntShowShare();
	void CntSelectedDownloads();
	void CntMergeSettings();

	void Init();
	void Page(Ui::Tags::Type);
	Ui::Prefs& GetPrefs();

	template <typename T>
	void Put(const std::string& key, const T& val) { page_.Put(key, val); }

	const Ui::Command& command_;
	Ui::Template page_;
};	

#endif //WEBUICONTROL_HPP


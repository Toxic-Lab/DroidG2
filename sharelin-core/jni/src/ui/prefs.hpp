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

#ifndef UI_PREFS_HPP
#define UI_PREFS_HPP

#include <map>

#include "sortprefs.hpp"
#include "tag.hpp"

namespace Ui {

class Prefs
{
public:
	SortPrefs sorting;
	std::map<Tags::Type, unsigned int> reloadRate;
	bool expandDownloads;	
};

} //namespace ui

#endif //UI_PREFS_HPP


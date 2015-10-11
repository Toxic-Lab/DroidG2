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

#include "contenttype.hpp"
#include <algorithm>
#include <cstring>

namespace ContentType {

inline bool Match(const char* category, const char* ext)
{
	return std::strstr(category, ext) != NULL;
}

std::string Resolve(const std::string& ext)
{
	return Resolve(ext.c_str());
}

std::string Resolve(const char* ext)
{
	const char image_png[] = ".png";
	const char text_html[] = ".css";

	const int MAX = 255;
	char e[MAX + 1];
	int i = 0;
	while(ext[i] && i < MAX)
	{
		e[i] = std::tolower(ext[i]);
		++i;
	}
	e[i] = 0;

	if(Match(image_png, e)) return "image/png";
	else if(Match(text_html, e)) return "text/html";
	else return "application/octet-stream";
}


} //namespace ContentType


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

#ifndef URL_HPP
#define URL_HPP

#include <string>

namespace Url {

struct Locator 
{
	void Clear();
	std::string Format() const;
	std::string FormatQuery() const;
	bool FromString(std::string str);

	std::string proto;
	std::string host;
	std::string uri;
	std::string query;
	unsigned int port;
};

std::string Encode(const std::string& str);
std::string Decode(const std::string& str);

} //namespace Url

#endif //URL_HPP



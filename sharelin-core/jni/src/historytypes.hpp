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

#ifndef HISTORY_TYPES_HPP
#define HISTORY_TYPES_HPP

#include <string>
#include "types.hpp"
#include "ip.hpp"

namespace History {

struct Download
{
	Download(): size(0) {}
	Download(const std::string& p, file_offset_t s): path(p), size(s) {}
	std::string path;
	file_offset_t size;
};

struct Upload
{
	std::string nick;
	std::string path;
	std::string client;
	file_offset_t sent;
	Ip::Endpoint addr;
};

} //namespace History

#endif //HISTORY_TYPES_HPP



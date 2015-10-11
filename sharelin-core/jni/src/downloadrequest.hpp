/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010 Andrey Stroganov <savthe@gmail.com>

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

#ifndef DOWNLOADREQUEST_HPP
#define DOWNLOADREQUEST_HPP

struct DownloadRequest
{
	Hashes::SHA1 sha1;
	Hashes::TTR ttr;
	Hashes::MD5 md5;
	Hashes::ED2K ed2k;
	file_offset_t size;
	std::string name;
};

#endif //DOWNLOADREQUEST_HPP

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

#include "sourcehost.hpp"

bool SourceHost::FromUri(std::string u)
{
	try
	{
		if(u.substr(0, 7) != "http://") return false;
		u.erase(0,7);
		const std::size_t i = u.find('/');
		if(!endpoint.FromString(u.substr(0, i))) return false;
		uri = u.substr(i);
	}
	catch(std::exception&)
	{
		return false;
	}

	return true;
}


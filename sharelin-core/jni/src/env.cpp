/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008  Andrey Stroganov

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

#include "env.hpp"
#include <cstdlib>
#include "sl.hpp"

std::string Env::Get(const std::string& var)
{
	std::string value;
	const char* s = std::getenv(var.c_str());
	if(s != NULL)
		value = s;
	return value;
}

std::string Env::HomeFolder()
{
	return std::string(root_directory);
	//return Env::Get("HOME");
}

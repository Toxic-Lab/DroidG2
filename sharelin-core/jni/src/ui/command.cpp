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

#include "command.hpp" 
#include <boost/algorithm/string.hpp>
#include "../url.hpp"

namespace Ui {

inline std::string ReadStringItem(const char*& p, char sep) 
{
	while(*p && *p == sep) ++p; //Skip spaces
	std::string s;
	if(*p)
	{
		bool subtext = *p == '"';
	
		if(subtext) ++p;

		//while not end of string and *p != separator 
		while(*p) 
		{
			if(*p == '\\' && *(p + 1)) //if backslash, use the symbol after it
				s += *(++p);
			else s += *p;
		
			++p;
		
			if(*p == 0) break;
		
			if(subtext && *p == '"') 
			{
				++p;
				break;
			}
		
			if(!subtext && (*p == sep || *p == '='))
				break;
		}
	
		while(*p && *p == sep) ++p; //Skip spaces
	}
	
	return s;
}

void Command::FromString(const std::string& sCmd, const std::string& sParams, char sep)
{
	if(sCmd.empty())
		throw std::runtime_error("Command is empty");
	if(sCmd.size() > 8)
		throw std::runtime_error("Command name is too large");

	command_ = StaticName::FromString(boost::to_lower_copy(sCmd));

	container_.clear();

	const char* p = sParams.c_str();

	while(*p)
	{
		std::string strParam = ReadStringItem(p, sep);
		if(strParam.empty())
			throw std::runtime_error("Parameter name is empty");

		std::string strVal;
		if(*p && *p == '=')
		{
			++p;
			if(*p)
			{
				if(*p != sep)
					strVal = ReadStringItem(p, sep);
				else
					++p;
			}
		}

		container_[boost::to_lower_copy(strParam)] = Url::Decode(strVal);
	}
}

Command::Iterator Command::Select(const std::string& key) const
{
	Command::Iterator i = Find(key);
	if(i == End()) throw NotFound(key);
	return i;
}

void Command::FromString(const std::string& s, char sep)
{
	if(s.empty()) throw std::runtime_error("Empty string supplied");

	std::string::size_type pos = s.find(sep);

	if(pos < s.size())
		FromString(s.substr(0, pos), s.substr(pos + 1), sep);
	else 
	{
		if(s.size() > 8) throw std::runtime_error("Command name is too large");
		command_ = StaticName::FromString(boost::to_lower_copy(s));
	}
}

bool Command::Has(const std::string& key) const
{
	return container_.find(key) != container_.end();
}

bool Command::Has(Tags::Type tag) const
{
	return Has(StaticName::ToString(tag));
}

bool Command::IsFlag(const std::string& key) const
{
	return Select(key)->second.empty();
}

bool Command::IsFlag(Tags::Type tag) const
{
	return IsFlag(StaticName::ToString(tag));
}

} //namespace ui

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

#include "url.hpp"
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>

namespace {

unsigned char HexToByte(char c)
{
	switch(c)
	{
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': case 'A': return 10;
		case 'b': case 'B': return 11;
		case 'c': case 'C': return 12;
		case 'd': case 'D': return 13;
		case 'e': case 'E': return 14;
		case 'f': case 'F': return 15;
	}
	return 0xff;
}

}

namespace Url {

std::string Locator::Format() const
{
	std::stringstream out;
	if(!proto.empty()) out << proto << "://";
	out << host;
	if(port) out << ":" << port;
	out << "/";
	if(!uri.empty()) out << uri;
	if(!query.empty()) out << "?" << query;
	return out.str();
}

std::string Locator::FormatQuery() const
{
	std::stringstream out;
	out << "/";
	if(!uri.empty()) out << uri;
	if(!query.empty()) out << "?" << query;
	return out.str();
}

bool Locator::FromString(std::string str)
{
	char debug[200];
	Clear();
	sprintf(debug, "Decoding url %s", str.c_str());
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	typedef std::string::size_type StrPos; 
	const StrPos npos = std::string::npos;
	
	StrPos i = str.find("://");
	if(i != npos) 
	{
		proto = str.substr(0, i);
		i += 3;
	}
	else i = 0;
	
	StrPos pos_slash = str.find("/", i);
	if(pos_slash == npos)
	{
		pos_slash = str.size();
		str.push_back('/');
	}
	StrPos pos_port = str.find(":", i);
	if(pos_port != npos)
	{
		if(pos_port > pos_slash) 
			return false;
		port = atoi(str.substr(pos_port + 1, pos_slash - pos_port - 1).c_str());
	}
	else
		host = str.substr(i, pos_slash - i);
	
	if(pos_slash == str.size() - 1)
		return true;
	std::string tmp = str.substr(pos_slash + 1);
	StrPos sep = tmp.find("?");
	uri = tmp.substr(0, sep);
	if(sep != npos)
		query = tmp.substr(sep + 1);
	
	uri = Decode(uri);
	query = Decode(query);

	//sprintf(debug, "loc debug %s %s", host.c_str(), uri.c_str());
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", debug);
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "returning true");
	return true;
}

void Locator::Clear()
{
	proto.clear();
	port = 0;
	host.clear();
	query.clear();
	uri.clear();
}

std::string Encode(const std::string& str)
{
	std::stringstream out;
	const std::string digits = "0123456789ABCDEF";
	for(size_t i = 0; i < str.size(); ++i)
	{
		const char c = str[i];
		if(str[i] == ' ') out << '+';
		else if (isalnum(c) || strchr("-_.!~*'()", c)) out << c;
		else out << "%" << digits[(c >> 4) & 0x0F] << digits[c & 0x0F];
	}
	return out.str();
}

std::string Decode(const std::string& str)
{
	std::stringstream in(str);
	std::stringstream out;
	char c;
	while(in >> c)
	{
		if(c == '+') out << ' ';
		else if(c == '%')
		{
			char a, b;
			if(in >> a >> b)
				out << char((HexToByte(a) << 4) | HexToByte(b));
		}
		else out << c;
	}
	return out.str();
}

} //namespace Url

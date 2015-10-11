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

#ifndef QUERYLANG_HPP
#define QUERYLANG_HPP

#include <cctype>
#include <string>
#include <boost/algorithm/string.hpp>

namespace G2 {

template <class OutIter>
void ExtractKeywords(const std::string& s, OutIter out)
{
	std::size_t i = 0;
	while(i < s.size())
	{
		while(i < s.size() && !std::isalnum(s[i])) ++i;
		const std::size_t w = i;

		while(i < s.size() && std::isalnum(s[i])) ++i;
		if(i - w >= 3) *out++ = boost::to_lower_copy(s.substr(w, i - w));
	}
}

template <class IncludeIter, class ExcludeIter>
void KeywordsFromQuery(const std::string& s, IncludeIter include, ExcludeIter exclude)
{
	if(s.empty()) return;

	std::size_t i = 0;

	while(i < s.size()) 
	{
		char flag;

		while(i < s.size() && s[i] == ' ') ++i;

		if(i == s.size()) break;

		if(s[i] == '-' || s[i] == '+') 
		{
			flag = s[i];
			if(++i == s.size()) break;
		} 
		else flag = '+';

		int j = i;
		if(s[i] == '"') 
		{
			if(++i == s.size()) break;
			while(i < s.size() && s[i] != '"') ++i;
		}
		else
			while(i < s.size() && s[i] != ' ') ++i;

		if(i - j < 3) continue;

		const std::string tmp = s.substr(j, i - j);

		if(i - j >= 3)
		{
			if(flag == '+') *include++ = tmp;
			else if(flag == '-') *exclude++ = tmp;
			else assert(!"could not be here");
		}
			
		if(i < s.size() && s[i] == '"') ++i; //closing "
	}
}

} //namespace G2

#endif //QUERYLANG_HPP


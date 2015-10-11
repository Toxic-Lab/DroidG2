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

#ifndef KWCACHE_HPP
#define KWCACHE_HPP

#include <map>
#include <set>
#include <string>
#include <functional>
#include <algorithm>
#include "querylang.hpp"
#include "foreach.hpp"

namespace Share {

template <class T>
class KeywordCache
{
	typedef std::set<std::string> Keywords;
	typedef std::set<T> Keys;
	typedef std::map<std::string, Keys> Cache;
	typedef std::map<T, Keywords> Table;

	Cache cache_;
	Table table_;

	struct Contains: public std::unary_function<T, bool>
	{
		Contains(const Table& table, const std::string& kw): table_(table), kw_(kw) {}

		bool operator()(const T& test) const
		{
			typename Table::const_iterator ikws = table_.find(test);
			assert(ikws != table_.end());
			return ikws->second.find(kw_) != ikws->second.end();
		}

		const Table& table_;
		std::string kw_;
	};

	template<class C>
	struct SizeCompare
	{
		bool operator()(const C& e1, const C& e2)
		{
			return e1.size() < e2.size();
		}
	};

public:
	std::size_t Size() const { return table_.size(); }
	
	void Add(const T& key, const std::string& str)
	{
		Remove(key);
		typename Table::iterator i = table_.insert(std::make_pair(key, Keywords())).first;

		G2::ExtractKeywords(str, std::inserter(i->second, i->second.begin()));

		if(i->second.empty()) Remove(key);
		else foreach(const std::string& kw, i->second) cache_[kw].insert(key);
	}

	void Remove(const T& key)
	{
		typename Table::iterator i = table_.find(key);
		if(i == table_.end()) return;

		foreach(const std::string& kw, i->second)
		{
			typename Cache::iterator ikw = cache_.find(kw);
			assert(ikw != cache_.end());

			ikw->second.erase(key);

			if(ikw->second.empty()) cache_.erase(ikw);
		}

		table_.erase(i);
	}

	template <class OutIter>
	void Find(const std::string& dn, OutIter out) const
	{
		Keywords include;
		Keywords exclude;
		G2::KeywordsFromQuery(dn, std::inserter(include, include.begin()), std::inserter(exclude, exclude.begin()));
		
		std::size_t sum = 0;
		for(Keywords::iterator i = include.begin(); i != include.end(); ++i) sum += i->size();
		if(sum < 5) return;

		if(include.empty()) return;

		typename Keywords::const_iterator ikw = std::max_element(include.begin(), include.end(), SizeCompare<std::string>());
		typename Cache::const_iterator ikeys = cache_.find(*ikw);
		if(ikeys == cache_.end()) return;
		include.erase(ikw);

		std::list <T> results;
		std::copy(ikeys->second.begin(), ikeys->second.end(), std::back_inserter(results));

		foreach(const std::string& kw, include)
			results.erase(std::remove_if(results.begin(), results.end(), std::not1(Contains(table_, kw))), results.end());
		foreach(const std::string& kw, exclude)
			results.erase(std::remove_if(results.begin(), results.end(), Contains(table_, kw)), results.end());
		
		std::copy(results.begin(), results.end(), out);
	}

};

} //namespace Share
#endif //KWCACHE_HPP


/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009  Andrey Stroganov <savthe@gmail.com>

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

#ifndef SEARCH_SEARCHER_HPP 
#define SEARCH_SEARCHER_HPP

#include <vector>
#include "types.hpp"
#include <map>
#include <string>
#include "media.hpp"
#include "ip.hpp"
#include <set>
#include <list>
#include "guid.hpp"
#include "g2packet.hpp"
#include <ctime>
#include "searchdefs.hpp"

namespace Search {

class Searcher
{
public:
	Searcher(const Search::Criteria& criteria, uint rate, GUID guid, ID id);
	void OnHit(const G2::Packet& pk, Ip::Endpoint fromEndpoint);
	bool Searched(Ip::Address);
	bool AddSearched(Ip::Address);
	void BuildQuery(G2::Packet& packet, Ip::Endpoint endpoint, uint32 qkey);
	void BuildQuery(G2::Packet& packet);
	void SetRate(uint rate);
	std::time_t SearchedTime() const { return timestamp_; }
	void SetSearchedTime(std::time_t t) { timestamp_ = t; }
	Criteria GetCriteria() const;
	Result GetResult(ID) const;
	ID GetID();
	uint Rate() { return rate_; }
	std::size_t CountResults() { return results_.size(); }

	template <typename OutIter>
	void DumpResults(OutIter i)
	{
		std::copy(results_.begin(), results_.end(), i);
	}

	void ClearResults() { results_.clear(); }


	typedef std::vector<Result> ResultsContainer;
	typedef std::map<Ip::Address, std::time_t> HubsContainer;


private:
	Criteria criteria_;
	uint rate_;
	GUID guid_;
	ID id_;
	std::time_t timestamp_;
	HubsContainer searchedHubs_;
	ResultsContainer results_;
	std::set<GUID> searchedGuids_;
};

} //namespace Search

#endif //SEARCH_SEEKER_HPP


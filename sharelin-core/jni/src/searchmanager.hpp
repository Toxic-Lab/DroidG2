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

#ifndef SEARCHMANAGER_HPP
#define SEARCHMANAGER_HPP

#include <boost/shared_ptr.hpp>
#include <list>
#include <stdexcept>
#include <map>
#include <vector>
#include "idgen.hpp"
#include "ip.hpp"
#include "period.hpp"
#include "g2packet.hpp"
#include "g2nodeinfo.hpp"
#include "searchdefs.hpp"

namespace Search {

class Searcher;
class Criteria;

class Manager
{
public:
	typedef boost::shared_ptr<Searcher> SearcherPtr;
	typedef std::map<GUID, SearcherPtr> SearchersContainer;
	enum Type {USER, AUTO};

	class Error: public std::runtime_error
	{
		public:
		Error(const std::string& err): std::runtime_error(err) {}
	};

	template <typename T>
	void DumpSearchers(T p)
	{
		for(SearchersContainer::iterator i = searchers_.begin();
				i != searchers_.end(); ++i)
			*p++ = i->second;
	}

	Manager();
	void Start();
	SearcherPtr Create(const Criteria& criteria, Type, uint rate = BASE_RATE*BASE_RATE);
	void Detach(SearcherPtr pSearcher);
	SearcherPtr Get(GUID guid);
	SearcherPtr Get(ID id);
	void OnTimer();
	void OnQKA(const G2::Packet& qka, Ip::Endpoint endpoint, Ip::Protocol::Type proto);
	void OnQH2(const G2::Packet& qh2, Ip::Endpoint endpoint, Ip::Protocol::Type proto);
	void OnQA(const G2::Packet& qh2, Ip::Endpoint endpoint, Ip::Protocol::Type proto);
	static std::time_t BaseRate() { return BASE_RATE; }
	unsigned CountUserSearches() const;
	void HubConnected(Ip::Endpoint);
	void HubDisconnected(Ip::Endpoint);

private:
	typedef std::map<Ip::Endpoint, std::time_t> NeighborsContainer;

	SearchersContainer::iterator Find(GUID guid);
	SearcherPtr SelectSearcher();
	bool LocalSearch(SearcherPtr);
	bool ForeignSearch(SearcherPtr);
	void DoSearch();
	void NeedMoreKeys();
	void RequestMoreKeys();

	SearchersContainer searchers_;
	Misc::IDGenerator idgen_;
	Time::Period periodSearch_;
	Time::Period periodQueryKey_;
	NeighborsContainer neighbors_;
	uint needKeys_;
};

} //namespace Search

#endif //SEARCHMANAGER_HPP


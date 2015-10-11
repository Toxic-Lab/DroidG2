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

#ifndef SORTPREFS_HPP
#define SORTPREFS_HPP

#include "sortoption.hpp"
#include <map>

namespace Ui {

class SortPrefs
{
public:
	SortPrefs(unsigned max = 500): max_(max) {}

	void Update(SortID id, SortID sortby)
	{
		if(options_.size() == max_) options_.clear();
		SortOption t = options_[id];
		t.Forward( t.SortBy() == sortby ? !t.Forward() : true );
		t.SortBy( sortby );
		options_[id] = t;
	}

	SortOption Option(SortID id) const 
	{
		Container::const_iterator i = options_.find(id);
		return i == options_.end() ? SortOption() : i->second;
	}

private:
	unsigned max_;
	typedef std::map<SortID, SortOption> Container;
	Container options_;
};

} //namespace Ui

#endif //SORTPREFS_HPP


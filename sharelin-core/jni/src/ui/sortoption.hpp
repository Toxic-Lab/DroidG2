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

#ifndef UI_SORTOPTION_HPP
#define UI_SORTOPTION_HPP

#include "../types.hpp"
#include "tag.hpp"

namespace Ui {

typedef Tags::Type SortID;

class SortOption
{
public:
	void SortBy(SortID sb) { sortby_ = sb; }
	void Forward(bool f) { forward_ = f; }
	SortID SortBy() const { return sortby_; }
	bool Forward() const { return forward_; }

private:
	SortID sortby_;
	bool forward_;
};

} //namespace Ui

#endif //UI_SORTOPTION_HPP


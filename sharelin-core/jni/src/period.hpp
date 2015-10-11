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

#ifndef PERIOD_HPP
#define PERIOD_HPP

#include <stdexcept>
#include "duration.hpp"

namespace Time {

class Duration;

enum InitWait {LONG_WAIT, DONT_WAIT, RAND_WAIT};

class Period
{
public:
	class GetTimeError: public std::runtime_error
	{
	public:
		GetTimeError(): std::runtime_error("Can't get time of day") {}
	};

	Period();
	Period(const Duration& duration, InitWait w = DONT_WAIT);
	void Set(const Duration& duration, InitWait w = DONT_WAIT);
	bool Set();

private:
	void Update();

	unsigned int sec_;
	unsigned int usec_;
	unsigned int nextSec_;
	unsigned int nextUsec_;
};

} //Time


#endif //PERIOD_HPP

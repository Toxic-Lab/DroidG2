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

#ifndef DURATION_HPP
#define DURATION_HPP

namespace Time {

class Duration
{
public:
	Duration(unsigned int ms): milliseconds(ms) {}
	unsigned int Get() const { return milliseconds; }
protected:
	unsigned int milliseconds;
};

class Milliseconds: public Duration
{
public:
	Milliseconds (unsigned int dur): Duration(dur) {}
};

class Seconds: public Duration
{
public:
	Seconds (unsigned int dur): Duration(dur * 1000) {}
};

class Minutes: public Duration
{
public:
	Minutes (unsigned int dur): Duration(dur * 1000 * 60) {}
};

class Hours: public Duration
{
public:
	Hours (unsigned int dur): Duration(dur* 1000 * 60 * 60) {}
};

} //namespace Time

#endif //DURATION_HPP


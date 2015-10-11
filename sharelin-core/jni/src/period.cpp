/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2007-2010  Andrey Stroganov <savthe@gmail.com>

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

#include <sys/time.h>
#include "period.hpp"
#include <stdlib.h>
#include <ctime>
#include <assert.h>

using namespace Time;

Period::Period()
{
	nextSec_ = 0;
	nextUsec_ = 0;
}

Period::Period(const Duration& duration, InitWait w)
{
	Set(duration, w);
}

void Period::Set(const Duration& duration, InitWait w)
{
	unsigned int ms = duration.Get();
	sec_ = ms / 1000;
	usec_ = (ms % 1000) * 1000;
	nextSec_ = nextUsec_ = 0;
	
	if(w == LONG_WAIT)
		Update();
	else if(w == RAND_WAIT)
	{
		Update();
		nextSec_ -= sec_ > 0 ? rand() % sec_ : 0;
		nextUsec_ -= nextUsec_ > 0 ? rand() % nextUsec_ : 0;
	}
}

void Period::Update()
{
	timeval tv;
	gettimeofday(&tv, 0);
	unsigned int x = tv.tv_usec + usec_;
	nextSec_ = tv.tv_sec + sec_ + x / (1000*1000);
	nextUsec_ = x % (1000*1000);
}

bool Period::Set()
{
	timeval tv;
	if(gettimeofday(&tv, 0) == -1)
		throw GetTimeError();
	assert(tv.tv_sec >= 0);
	assert(tv.tv_usec >= 0);
	if(unsigned(tv.tv_sec) < nextSec_) 
		return false;
	if(unsigned(tv.tv_sec) > nextSec_ || unsigned(tv.tv_usec) > nextUsec_)
	{
		Update();
		return true;
	}
	return false;
}



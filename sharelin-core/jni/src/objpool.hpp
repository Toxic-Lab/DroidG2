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

#ifndef OBJPOOL_HPP
#define OBJPOOL_HPP

#include <list>
#include <iostream>

template <class T> 
class ObjectsPool
{
public:
	virtual ~ObjectsPool()
	{
		for(typename std::list<T*>::iterator i = pool_.begin(); i != pool_.end(); ++i)
			delete *i;
	}

	virtual T* Get()
	{
		if(pool_.empty()) 
		{
		//	std::cout << "allocation" << std::endl;
			return new T();
		}
		//std::cout << "exists: " << pool_.size() << std::endl;
		T* p = pool_.front();
		pool_.pop_front();
		return p;
	}

	virtual void Release(T* p)
	{
		pool_.push_back(p);
		//std::cout << "released. Buffers: " << pool_.size() << std::endl;
	}

protected:
	std::list<T*> pool_;
};

#endif //OBJPOOL_HPP


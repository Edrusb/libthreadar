/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2020 Denis Corbin
//
// This file is part of libthreadar
//
//  libthreadar is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  libhtreadar is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with libthreadar.  If not, see <http://www.gnu.org/licenses/>
//
//----
//  to contact the author: dar.linux@free.fr
/*********************************************************************/

#include <libthreadar/libthreadar.hpp>

const int cond_full = 0;
const int cond_empty = 1;
condition verrou(2);
    // two instances will be available, instance 0 and instance 1
    // using the scemantics of the const variable just above

const int resource_max_size = 10;
std::deque<int> resource;


void thread1::routine()
{
    while(true)
    {
	    // thread1 is fetching data from the resource

	verrou.lock(); // obtaining exclusivity and right to manipulate the resource

	if(resource.empty())
	    verrou.wait(cond_empty); // temporary releasing the resource because it is empty

	    // using the resource now we know it is no (more) empty
	cout << resource.front(); << endl;
	resource.pop_front();

	    // resource is no more full as we have consumed one entry from it
	    // we need to awake thread2 if it was pending for the resource to free up
	    // some space:

	verrou.signal(cond_full);
	    // this will release thread2 but only once we
	    // our call to verrou.unlock() will have returned:

	verrou.unlock(); // release the exclusivity and right of using the resource
    }
}

void thread2::routine()
{
    int counter = 0;

    while(true)
    {
	    // thread 2 feeds the resource

	verrou.lock();
	if(resource.size() > resource_max_size)
	    verrou.wait(cond_full);
	resource.push_back(++counter);
	verrou.signal(cond_empty);
	verrou.unlock();
    }
}

/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2025 Denis Corbin
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

#include "config.h"

    // C system headers
extern "C"
{
}
    // C++ standard headers
#include <string>

    // libthreadar headers

    // this module's header
#include "condition.hpp"

using namespace std;

namespace libthreadar
{

    condition::condition(unsigned int num): cond(num), counter(num)
    {
	if(num < 1)
	    throw exception_range("need at least one instance to create a condition object");

	for(unsigned int i = 0; i < num; ++i)
	{
	    int ret = pthread_cond_init(&(cond[i]), NULL);
	    if(ret != 0)
	    {
		for(signed int dec = i - 1; dec >= 0; --dec)
		    (void)pthread_cond_destroy(&(cond[dec]));
		throw string("Error while creating condition");
	    }
	    counter[i] = 0;
	}
    }

    condition::~condition()
    {
	std::deque<pthread_cond_t>::iterator it = cond.begin();

	while(it != cond.end())
	{
	    (void)pthread_cond_destroy(&(*it));
	    ++it;
	}
    }

    void condition::wait(unsigned int instance)
    {
	if(instance < cond.size())
	{
	    ++counter[instance];
	    int ret = pthread_cond_wait(&(cond[instance]), &mut);
	    --counter[instance];
	    if(ret != 0)
		throw string("Error while going to wait on condition");
	}
	else
	    throw exception_range("the instance number given to condition::wait() is out of range");
    }

    void condition::signal(unsigned int instance)
    {
	if(instance < cond.size())
	{
	    int ret = pthread_cond_signal(&(cond[instance]));
	    if(ret != 0)
		throw string("Error while unlocking and signaling");
	}
	else
	    throw exception_range("the instance number given to condition::signal() is out of range");
    }

    void condition::broadcast(unsigned int instance)
    {
	if(instance < cond.size())
	{
	    int ret = pthread_cond_broadcast(&(cond[instance]));
	    if(ret != 0)
		throw string("Error while unlocking and broadcasting");
	}
	else
	    throw exception_range("the instance number given to condition::broadcast() is out of range");
    }


} // end of namespace

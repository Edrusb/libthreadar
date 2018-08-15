/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2018 Denis Corbin
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


    // libthreadar headers
#include "exceptions.hpp"

    // this module's header
#include "semaphore.hpp"

using namespace std;

namespace libthreadar
{


    semaphore::semaphore(unsigned int max_val) : max_value(max_val)
    {
	value = max_val;
	semaph.lock(); // now semaph will suspend thread for which lock() is called
    }

    semaphore::~semaphore()
    {
	reset();
	semaph.unlock();
    }

    bool semaphore::waiting_thread() const
    {
	return value < 0; // reading of integer is atomic CPU single operation, no need to lock val_mutex
    }

    bool semaphore::working_thread() const
    {
	return value < max_value; // reading of integer is atomic CPU single operation, no need to lock val_mutex
    }

    void semaphore::lock()
    {
	bool locking = false;

	val_mutex.lock();
	--value;
	if(value < 0)
	    locking = true;
	val_mutex.unlock();

	if(locking)
	    semaph.lock();
    }

    void semaphore::unlock()
    {
	bool unlocking = false;

	val_mutex.lock();
	try
	{
	    if(value == max_value)
		throw exception_range("too much call to unlock() given the number of lock() so far");
	    ++value;
	    if(value <= 0)
		unlocking = true;
	}
	catch(...)
	{
	    val_mutex.unlock();
	    throw;
	}
	val_mutex.unlock();

	if(unlocking) // value was negative at the beginning of this call, meaning at least one thread was waiting
	    semaph.unlock();
    }

    void semaphore::reset()
    {
	val_mutex.lock();
	try
	{
	    while(value < 0)
	    {
		semaph.unlock();
		++value;
	    }
	    value = max_value;
	}
	catch(...)
	{
	    val_mutex.unlock();
	    throw;
	}
	val_mutex.unlock();
    }


} // end of namespace

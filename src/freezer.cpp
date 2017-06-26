/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2015 Denis Corbin
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
#include "freezer.hpp"

using namespace std;

namespace libthreadar
{


    freezer::freezer()
    {
	value = 0;
	semaph.lock(); // now semaph will suspend thread for which lock() is called
    }

    freezer::~freezer()
    {
	reset();
	semaph.unlock();
    }

    bool freezer::waiting_thread() const
    {
	return value < 0; // reading of integer is atomic CPU single operation, no need to lock val_mutex
    }

    void freezer::lock()
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

    void freezer::unlock()
    {
	bool unlocking = false;

	val_mutex.lock();
	try
	{
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

    void freezer::reset()
    {
	val_mutex.lock();
	try
	{
	    while(value < 0)
	    {
		semaph.unlock();
		++value;
	    }
	    value = 0;
	}
	catch(...)
	{
	    val_mutex.unlock();
	    throw;
	}
	val_mutex.unlock();
    }


} // end of namespace

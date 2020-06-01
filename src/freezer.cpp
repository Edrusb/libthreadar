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
    }

    freezer::~freezer()
    {
	reset();
    }

    bool freezer::waiting_thread() const
    {
	return value < 0; // reading of integer is atomic CPU operation, no need to lock val_mutex
    }

    void freezer::lock()
    {
	cond.lock();
	try
	{
	    --value;
	    if(value < 0)
		cond.wait();
	}
	catch(...)
	{
	    cond.unlock();
	    throw;
	}
	cond.unlock();
    }

    void freezer::unlock()
    {
	cond.lock();
	try
	{
	    ++value;
	    if(value <= 0)
		cond.signal();
	}
	catch(...)
	{
	    cond.unlock();
	    throw;
	}
	cond.unlock();
    }

    void freezer::reset()
    {
	bool loop = true;

	do
	{
	    cond.lock();
	    try
	    {
		if(value < 0)
		{
		    ++value;
		    cond.signal();
		}
		else
		{
		    value = 0;
		    loop = false;
		}
	    }
	    catch(...)
	    {
		cond.unlock();
		throw;
	    }
	    cond.unlock();
	}
	while(loop);
    }


} // end of namespace

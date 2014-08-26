/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014 Denis Corbin
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
#include "barrier.hpp"

using namespace std;

namespace libthreadar
{

    barrier::barrier(unsigned int num)
    {
	switch(pthread_barrier_init(&bar, NULL, num))
	{
	case 0:
	    break;
	case EAGAIN:
	    throw exception_range("Lack of resource");
	case EINVAL:
	    throw exception_range("zero given as argumet to barrier");
	case ENOMEM:
	    throw exception_memory();
	case EBUSY:
	    throw THREADAR_BUG;
	default:
	    throw THREADAR_BUG;
	}
    }

    barrier::~barrier()
    {
	switch(pthread_barrier_destroy(&bar))
	{
	case 0:
	    break;
	case EBUSY:
	    throw exception_range("destroying a barrier while still in use");
	case EINVAL:
	    throw THREADAR_BUG;
	default:
	    throw THREADAR_BUG;
	}
    }

    void barrier::wait()
    {
	switch(pthread_barrier_wait(&bar))
	{
	case PTHREAD_BARRIER_SERIAL_THREAD:
	    break;
	case 0:
	    break;
	case EINVAL:
	    throw THREADAR_BUG;
	default:
	    throw THREADAR_BUG;
	}
    }

} // end of namespace


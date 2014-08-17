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
#if HAVE_ERRNO_H
#include <errno.h>
#endif
}
    // C++ standard headers


    // libthreadar headers

    // this module's header
#include "mutex.hpp"

using namespace std;

namespace libthreadar
{

    mutex::mutex()
    {
	int ret = pthread_mutex_init(&mut, NULL);
	if(ret != 0)
	    throw string("Error while creating mutex"); // ret should be reported with exception
    }

    mutex::~mutex()
    {
	(void)pthread_mutex_destroy(&mut);
    }

    void mutex::lock()
    {
	switch(pthread_mutex_lock(&mut))
	{
	case 0:
	    break;
	case EINVAL:
	    throw string("BUG");
	case EDEADLK:
	    throw string("BUG");
	case EPERM:
	    throw string("BUG");
	default:
	    throw string("BUG");
	}
    }

    void mutex::unlock()
    {
	if(pthread_mutex_unlock(&mut) != 0)
	    throw string("BUG");
    }


    bool mutex::try_lock()
    {
	int ret = pthread_mutex_trylock(&mut);
	if(ret != 0 && ret != EBUSY)
	    throw string("Error while trying locking mutex"); // ret should be reported with exception

	return ret == 0;
    }

} // end of namespace


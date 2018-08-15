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
#include <string>

    // libthreadar headers

    // this module's header
#include "condition.hpp"

using namespace std;

namespace libthreadar
{

    condition::condition()
    {
	int ret = pthread_cond_init(&cond, NULL);
	if(ret != 0)
	    throw string("Error while creating condition");
    }

    condition::~condition()
    {
	(void)pthread_cond_destroy(&cond);
    }

    void condition::wait()
    {
	int ret = pthread_cond_wait(&cond, &mut);
	if(ret != 0)
	    throw string("Error while going to wait on condition");
    }

    void condition::signal()
    {
	try
	{
	    int ret = pthread_cond_signal(&cond);
	    if(ret != 0)
		throw string("Error while unlocking and signaling");
	}
	catch(...)
	{
	    unlock();
	    throw;
	}
    }

} // end of namespace

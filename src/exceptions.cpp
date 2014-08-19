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

    // this module's header
#include "exceptions.hpp"

using namespace std;

namespace libthreadar
{

    exception_system::exception_system(const std::string & context, int error_code) : exception_base("")
    {
	const unsigned int MSGSIZE = 300;
	char buffer[MSGSIZE];

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
	    // we expect the XSI-compliant strerror_r
	int val = strerror_r(error_code, buffer, MSGSIZE);
	if(val != 0)
	    strncpy(buffer, "Error code to message conversion, failed", MSGSIZE);
#else
	char *val = strerror_r(error_code, buffer, MSGSIZE);
	if(val != buffer)
	    strncpy(buffer, val, MSGSIZE);
#endif
	buffer[MSGSIZE-1] = '\0';

	reset_first_message(buffer);
	push_message(context);
    }


} // end of namespace


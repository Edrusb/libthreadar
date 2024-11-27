/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2024 Denis Corbin
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
#include <sstream>

    // libthreadar headers

    // this module's header
#include "exceptions.hpp"

using namespace std;

namespace libthreadar
{
    static string tools_strerror_r(int errnum);

    string exception_base::get_message(const string & sep) const
    {
	string ret = "";

	if(msg_table.size() > 0)
	    ret = msg_table[0];

        for(unsigned int i = 1; i < size(); ++i)
	    ret += sep + msg_table[i];

	return ret;
    }

    exception_system::exception_system(const std::string & context, int error_code) : exception_base("")
    {
	push_message(context);
	push_message(tools_strerror_r(error_code));
    }

	// borrowing from libdar tools.cpp
	//

    string tools_int2str(signed int x)
    {
        ostringstream tmp;

        tmp << x;

        return tmp.str();
    }

#define MSGSIZE 200

    static string tools_strerror_r(int errnum)
    {
        char buffer[MSGSIZE];
        string ret;

#ifdef HAVE_STRERROR_R
#ifdef HAVE_STRERROR_R_CHAR_PTR
        char *val = strerror_r(errnum, buffer, MSGSIZE);
        if(val != buffer)
            strncpy(buffer, val, MSGSIZE);
#else
            // we expect the XSI-compliant strerror_r
        int val = strerror_r(errnum, buffer, MSGSIZE);
        if(val != 0)
	{
	    string tmp = "failed converting to message the error code " + tools_int2str(errnum);
            strncpy(buffer, tmp.c_str(), tmp.size()+1 < MSGSIZE ? tmp.size()+1 : MSGSIZE);
	}
#endif
#else
	char *tmp = strerror(errnum);
	(void)strncpy(buffer, tmp, MSGSIZE);
#endif
        buffer[MSGSIZE-1] = '\0';
        ret = buffer;

        return ret;
    }

	//
	// borrowing end

} // end of namespace


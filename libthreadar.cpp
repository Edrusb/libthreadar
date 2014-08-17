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
#include "libthreadar.hpp"

const unsigned int LIBTHREADAR_MAJOR = 1;
const unsigned int LIBTHREADAR_MEDIUM = 0;
const unsigned int LIBTHREADAR_MINOR = 0;

namespace libthreadar
{

    void get_version(U_I & major, U_I & medium, U_I & minor)
    {
	major = LIBTHREADAR_MAJOR;
	medium = LIBTHREADAR_MEDIUM;
	minor = LIBTHREADAR_MINOR;
    }

} // end of namespace

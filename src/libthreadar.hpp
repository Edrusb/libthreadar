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

#ifndef LIBTHREADAR_HPP
#define LIBTHREADAR_HPP

    /// \file libthreadar.hpp
    /// \brief only that file should be included from libthreadar
    ///
    /// libthreadar provide access to different C++ classes:
    //. class mutex
    //. class semaphore
    //. class tampon (i.e.: buffer)
    //. class thread

#include "config.h"

#include "mutex.hpp"
#include "semaphore.hpp"
#include "tampon.hpp"
#include "thread.hpp"

namespace libthreadar
{

	/// provides the version of libthreadar
    extern void get_version(unsigned int & major, unsigned int & medium, unsigned int & minor);

} // end of namespace

#endif

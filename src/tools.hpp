/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2025 Denis Corbin
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

#ifndef LIBTHREADAR_TOOLS_HPP
#define LIBTHREADAR_TOOLS_HPP

    /// \file tools.hpp
    /// \brief defines tools used inside libthreadar

#include "config.h"

    // C system headers
extern "C"
{
}
    // C++ standard headers
#include <sstream>

    // libthreadar headers

namespace libthreadar
{
    template <class T> std::string tools_convert_to_string(T val)
    {
	std::stringstream tmp;
	tmp << val;
	return tmp.str();
    }

} // end of namespace

#endif

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

#ifndef LIBTHREADAR_HPP
#define LIBTHREADAR_HPP

    /// \file libthreadar.hpp
    /// \brief main header file of the library, only that file should be # included in your programs

    /// \mainpage
    /// This is the documentation pages of Libthreadar, a C++ library that provides several classes to manipulate threads:
    /// - class barrier
    /// - class mutex
    /// - class semaphore
    /// - class fast_tampon
    /// - class thread
    /// - class thread_signal
    /// - class freezer
    /// - class condition
    /// - class ratelier_gather
    /// - class ratelier_scatter
    /// .
    /// These classes are to be used independently from each others and are defined within the libthreadar namespace.
    /// An example of use "hello word" is available in the example subdirectory of source package.
    ///
    /// source code download, support, discussion, and so on is available from sourceforge page at https://sourceforge.net/projects/libthreadar/
    ///
#include "config.h"

#include "mutex.hpp"
#include "semaphore.hpp"
#include "condition.hpp"
#include "barrier.hpp"
#include "tampon.hpp"
#include "fast_tampon.hpp"
#include "thread.hpp"
#include "thread_signal.hpp"
#include "freezer.hpp"
#include "ratelier_gather.hpp"
#include "ratelier_scatter.hpp"

   /// This is the only namespace used in libthreadar and all symbols provided by libthreadar are member of this namespace.

namespace libthreadar
{

	/// provides the version of libthreadar

	/// \param[out] major is incremented when the API changes in a way it becomes incompatible
	/// with previous API so user program have to be adapted to
	/// be used with it else they might even not compile.
	/// \param[out] medium is incremented when new features are added to the library in a way
	/// it stays compatible with previous version of the API (user program should not need to be
	/// modified nor recompiled)
	/// \param[out] minor is incremented when bug fix is brought to the library without any new
	/// feature or feature enhancment nor API modification.
    extern void get_version(unsigned int & major, unsigned int & medium, unsigned int & minor);

} // end of namespace

extern "C"
{
	/// in case you use autoconf AC_CHECK_LIB in your program to detect the availability of libthreadar

	/// use AC_CHECK_LIB(threadar, [libthreadar_for_autoconf], [], [])
	/// to have autoconf based configure script properly detecting
	/// the presence and usability of libthreadar
    extern unsigned int libthreadar_for_autoconf(unsigned int x);
}

#endif

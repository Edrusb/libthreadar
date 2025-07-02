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
    /// \par Description
    /// This is the documentation pages of Libthreadar, a C++ library which provides several classes to manipulate threads:
    /// - \link libthreadar::barrier class barrier\endlink
    /// - \link libthreadar::freezer class freezer\endlink
    /// - \link libthreadar::mutex class mutex\endlink
    /// - \link libthreadar::semaphore class semaphore\endlink
    /// - \link libthreadar::fast_tampon class fast_tampon\endlink
    /// - \link libthreadar::thread class thread\endlink
    /// - \link libthreadar::thread_signal class thread_signal\endlink
    /// - \link libthreadar::freezer class freezer\endlink
    /// - \link libthreadar::condition class condition\endlink
    /// - \link libthreadar::ratelier_gather class ratelier_gather\endlink
    /// - \link libthreadar::ratelier_scatter class ratelier_scatter\endlink
    /// .
    /// These classes are independent from each others (even if some inherit from some others like libthreadar::condition from libthreadar::mutex)
    /// and are defined within the \ref libthreadar namespace.
    /// Examples of use are available in the example subdirectory of source package.
    ///
    /// \par Source code
    /// - GIT repository at <a href="https://sourceforge.net/p/libthreadar/code/">Sourceforge</a>
    /// - GIT repository at <a href="">Github</a> (both GIT repos should be synchronized most of the time)
    /// - packaged sources code at <a href="https://sourceforge.net/projects/libthreadar/files/">Sourceforge</a>
    /// All packages sources and tags in git repos should be signed. See the Author below section to authenticate signatures.
    ///
    /// \par Support
    /// support can be requested using the <a href="https://github.com/Edrusb/libthreadar/issues">issues</a> at github.
    ///
    /// \par Copyright
    /// Libthreadar library is licensed under the terms of the GNU Lesser General Public License v3.0
    /// see the <a href="https://github.com/Edrusb/libthreadar/blob/master/COPYING">COPYING</a>
    /// file in the source code for details
    ///
    /// \author All dar/libdar/webdar/libthreadar produced software packages should be authenticated using <a href="http://dar.linux.free.fr/doc/authentification.html">Denis's GPUPG signature</a>
    /// \par History and origin
    /// The code of libthreadar has been extracted from Webdar project near 2015, due to the fact it would be useful to dar/libdar
    /// to implment parallel compression and parallel encryption in particular (where from this project name), while Webdar
    /// was abandonned at that time (it has been ressurrected
    /// in 2022 and should get its first release in 2025).

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

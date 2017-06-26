/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2015 Denis Corbin
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

#ifndef LIBTHREADAR_MUTEX_HPP
#define LIBTHREADAR_MUTEX_HPP

    /// \file mutex.hpp
    /// \brief defines the mutex C++ class

#include "config.h"

    // C system headers
extern "C"
{
#if HAVE_PTHREAD_H
#include <pthread.h>
#endif
}
    // C++ standard headers
#include <string>


    // libthreadar headers



namespace libthreadar
{
	/// Wrapper around the Posix pthread_mutex_t C objects

	/// To protect a data against concurrent access by different threads
	/// each thread has to call the lock() method before and unlock() method
	/// after accessing that data.
	/// If another thread is already accessing that data calling lock() will
	/// suspended the calling thread up to the time the thread accessing the data calls unlock()
    class mutex
    {
    public:
	    /// constructor
	mutex();

	    // no copy constructor (made private)

	    // no assignment operator (made private)

	    /// destructor
	~mutex();

	    /// lock the mutex

	    /// \note if another thread has called lock() and not yet called unlock(),
	    /// the calling thread is suspended up to the time the mutex is
	    /// released by a call to unlock().
	void lock();

	    /// unlock the mutex

	    /// \note if one or more threads are suspended on that mutex, a single
	    /// thread suspended is then awaken and returns from its call to lock().
	void unlock();

	    /// Tells whether calling lock() would currently suspend the caller or not

	    /// \return true if lock is acquired false if mutex was already locked
	bool try_lock();

    private:
	mutex(const mutex & ref) { throw THREADAR_BUG; };
	const mutex & operator = (const mutex & ref) { throw THREADAR_BUG; };

	pthread_mutex_t mut; //< the mutex
    };

} // end of namespace

#endif

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

#ifndef LIBTHREADAR_BARRIER_HPP
#define LIBTHREADAR_BARRIER_HPP

    /// \file barrier.hpp
    /// \brief defines a barrier C++ class, to synchronize N threads

#include "config.h"

    // C system headers
extern "C"
{
#if HAVE_PTHREAD_H
#include <pthread.h>
#endif
}
    // C++ standard headers


    // libthreadar headers



namespace libthreadar
{
	/// the class barrier allow num threads to synchronize
	///
	/// the number of thread to synchronize is given in the constructor
	/// argument. All thread calling the wait() method get locked
	/// until 'num' thread have called this wait() method at which
	/// time they are all unlocked. The barrier is then ready for a
	/// new cycle
	/// \note, the barrier shall not be destroyed if at least one thread
	/// is waiting (locked) on it
    class barrier
    {
    public:
	barrier(unsigned int num);
	    // no copy constructor (made private)
	    // no assignment operator (made private)
	~barrier();

	    /// suspend the calling thread up to the time a total of 'num' (given to constructor) threads have also called wait(). Then all suspended thread are resumed
	void wait();

    private:
	barrier(const barrier & ref) { throw THREADAR_BUG; };
	const barrier & operator = (const barrier & ref) { throw THREADAR_BUG; };
	pthread_barrier_t bar;
    };

} // end of namespace

#endif

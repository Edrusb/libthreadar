/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2020 Denis Corbin
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
    /// \brief defines the barrier C++ class, to synchronize several threads

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
        /// the class barrier allows several threads to synchronize between them

        /// the number of thread to synchronize is given in the constructor
        /// argument 'num'. All thread calling the wait() method get locked
        /// until 'num' thread(s) have called this wait() method at which
        /// time they are all unlocked. The barrier object is then ready for a
        /// new cycle.
        /// \note The barrier shall not be destroyed if at least one thread
        /// is waiting (locked) on it
    class barrier
    {
    public:
            /// The constructor

            /// \param[in] num is the number of thread to synchronize
        barrier(unsigned int num);

            /// no copy constructor
        barrier(const barrier & ref) = delete;

            /// no move constructor
        barrier(barrier && ref) noexcept = default;

            /// no assignment operator
        barrier & operator = (const barrier & ref) = delete;

            /// no move operator
        barrier & operator = (barrier && ref) noexcept = default;

            /// The destructor

            /// \note A barrier object must not be destroyed if some thread are suspended calling wait() on it
        ~barrier() noexcept(false);

            /// suspend the calling thread waiting for other up to 'num' other thread to call wait too

            /// \note A thread is suspended up to the time a total amount of 'num' threads (as given to the barrier constructor)
            /// have also called wait(). Then all suspended thread are resumed.
        void wait();

	    /// return the barrier size
	unsigned int get_count() const { return val; };

	    /// return the number of thread waiting on the barrier or just released from it

	    /// \note this is to be seen as an approximation as a thread can be about to
	    /// be suspended but not yet counted, as well as a thread may be just released
	    /// while not yet removed from the count
	unsigned int get_waiting_count() const { return waiting_num; };

    private:
	pthread_barrier_t bar;
	unsigned int val;
	unsigned int waiting_num;
    };

        /// \example ../doc/examples/barrier_example.cpp

} // end of namespace

#endif

/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2018 Denis Corbin
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

#ifndef LIBTHREADAR_SEMAPHORE_HPP
#define LIBTHREADAR_SEMAPHORE_HPP

    /// \file semaphore.hpp
    /// \brief defines the semaphore class
    ///

#include "config.h"

    // C system headers
extern "C"
{

}
    // C++ standard headers
#include <string>


    // libthreadar headers
#include "mutex.hpp"

namespace libthreadar
{

	/// Class semaphore is an enhanced version of Posix semaphore

	/// In addition to Posix Semaphore, this class let the calling thread to detect whether other
	/// thread than the one currently having the lock are waiting for the semaphore to be released
    class semaphore
    {
    public:
	    /// semaphore constuctor

	    /// \param[in] max_value is the maximum number of thread that can concurrently request wait() without being suspended
	    /// \note the initial value is set to the max value, calling lock() can decrease this initial value at will, but
	    /// calling unlock() which increases the semaphore value that must not lead it to exceed maximum value. In other
	    /// words there must not be more unlock() calls than lock() calls invoked so far.
	    /// \note A thread calling lock() when the value is less than or equal to zero is suspended
	    /// up to the time another thread calls unlock(). If more than one was pending on that semaphore, unlock() awakes a single thread.
	semaphore(unsigned int max_value);

	    /// no copy constructor
	semaphore(const semaphore & ref) = delete;

	    /// no move constructor
	semaphore(semaphore && ref) noexcept = delete;

	    /// no assignment operator
	semaphore & operator = (const semaphore & ref) = delete;

	    /// no move operator
	semaphore & operator = (semaphore && ref) noexcept = delete;

	    /// Destructor
	~semaphore();

	    /// Return whether the semaphore has at least a pending thread waiting for another thread to unlock it
	bool waiting_thread() const;


	    /// return whether the semaphore has at least one thread that acquired the lock, possibily without other thread pending
	bool working_thread() const;

	    /// Request a "resource"

	    /// \note at most max_value (given at construction time) "resources" can be requested at a time.
	    /// if no more "resource" is available the caller is suspended waiting for another thread to release
	    /// a resource calling unlock(). The notion of "resource" is an abstraction, that's up to the developer
	    /// relying on that class to define what a "resource" is. The semaphore only assures that at most max_value
	    /// resource will be used at the same time.
	void lock();

	    /// Release a "resource"

	    /// \note Note that if one or more thread are suspended due to a call to lock() a single thread is awaken
	    /// and returns from the lock() call it was suspended into.
	void unlock();

	    /// Reset to initial state releasing any thread that could wait on the semaphore
	void reset();

	    /// Return the value of the semaphore, that's to say the number of available "resources".

	    /// \note a negative value gives the total number of threads that are suspended by calling lock() (well the opposit value of
	    /// the number of threads more precisely).
	int get_value() const { return value; };

    private:
	int value;            //< this is the semaphore value
	mutex val_mutex;      //< this controls modification to value
	mutex semaph;         //< this mutex is used to suspend thread semaphore value get negative
	const int max_value;  //< maximum value the semaphore cannot exceed
    };

} // end of namespace

#endif

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

#ifndef LIBTHREADAR_SEMAPHORE_HPP
#define LIBTHREADAR_SEMAPHORE_HPP

    /// \file semaphore.hpp
    /// \brief defines a semaphore class
    ///

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

	/// class semaphore is used as replacement of sem_t type in order to detect whether other
	/// thread than the one currently having the lock are waiting for the semaphore to be released
    class semaphore
    {
    public:
	    /// semaphore constuctor
	    ///
	    /// \param[in] max_value is the maximum number of thread that can concurrently request wait() without being suspended
	semaphore(unsigned int max_value);

	semaphore(const semaphore & ref):max_value(0) { throw std::string("BUG"); };
	const semaphore & operator = (const semaphore & ref) { throw std::string("BUG"); };
	~semaphore();

	    /// return whether the semaphore has at least a pending thread waiting for another thread to unlock it
	bool waiting_thread() const;


	    /// return whether the semaphore has at least one thread that acquired the lock, possibily without other thread pending
	bool working_thread() const;

	    /// request a "resource"
	    ///
	    /// \note at most max_value (given at construction time) "resources" can be requested at a time.
	    /// if no more "resource" is available the caller is suspended waiting for another thread releasing
	    /// a resource calling unlock(). The notion of "resource" is an abstraction, that's up to the developer
	    /// relying on that class to define what a "resource" is. The semaphore only assures that at most max_value
	    /// resource will be used at the same time.
	void lock();

	    /// release a "resource"
	    ///
	    /// \note that if one or more thread are suspended waiting during a call to lock() a single thread is awaken
	    /// and returns from the lock() call it was suspended into.
	void unlock();

	    /// reset to initial state releasing any thread that could wait on us
	void reset();

	    /// return the value of the semaphore, that's to say the number of available "resources".
	    ///
	    /// \note a negative value gives the total number of threads that are suspended by calling lock() (the opposit of the value
	    /// more precisely).
	int get_value() const { return value; };

    private:
	int value;            //< this is the semaphore value
	mutex val_mutex;      //< this controls modification to value
	mutex semaph;         //< this is mutex is used to suspend thread when more than one is requesting the lock
	const int max_value;  //< maximum value the semaphore cannot exceed
    };

} // end of namespace

#endif

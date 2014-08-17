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

#ifndef LIBTHREADAR_MUTEX_HPP
#define LIBTHREADAR_MUTEX_HPP

    /// \file mutex.hpp
    /// \brief defines a mutex C++ class, using pthread_mutex_t C object

    // C system headers
extern "C"
{
#include <pthread.h>
}
    // C++ standard headers
#include <string>


    // libthreadar headers



namespace libthreadar
{
	/// the class mutex allow control to data by a single thread
	///
	/// to protect a data against concurrent access by different threads
	/// each thread has to call the lock() method before and unlock() method
	/// after accessing that data.
	/// if another thread is already accessing that data the caller to lock()
	/// is suspended up to the time the thread accessing the data calls unlock()
    class mutex
    {
    public:
	mutex();
	mutex(const mutex & ref) { throw std::string("BUG"); };
	const mutex & operator = (const mutex & ref) { throw std::string("BUG"); };
	~mutex();

	    /// lock the mutex
	    ///
	    /// \note if another thread called lock() and not yet called unlock(),
	    /// this call puts the caller is suspended up to the time the mutex is
	    /// released by a call to unlock().
	void lock();

	    /// unlock the mutex
	    ///
	    /// \note if one or more threads are suspended on that mutex, a single
	    /// thread suspended is then awaken and returns from its call to lock().
	void unlock();

	    /// Tells whether calling lock() would currently suspend the caller or not
	    ///
	    /// \return true if lock is acquired false if mutex is already locked
	bool try_lock();

    private:
	pthread_mutex_t mut; //< the mutex
    };

} // end of namespace

#endif

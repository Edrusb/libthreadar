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

#ifndef CONDITION_HPP
#define CONDITION_HPP

    /// \file condition.hpp
    /// \brief defines the condition class

#include "mutex.hpp"
#include "exceptions.hpp"

namespace libthreadar
{

	/// Wrapper around the Posix pthread_cond_t object and its associated mutex

    class condition : public mutex
    {
    public:

	    /// constructor
	condition();

	    /// no copy constructor
	condition(const condition & ref) = delete;

	    /// no move constructor
	condition(condition && ref) noexcept = delete;

	    /// no assignment operator
	condition & operator = (const condition & ref) = delete;

	    /// no move operator
	condition & operator = (condition && ref) noexcept = delete;

	    /// destructor
	~condition();


	    /// put the calling thread on hold waiting for another thread to call signal()

	    /// \note wait() must be called between lock() and unlock(). Once suspendend, the
	    /// calling thread unlocks the mutex and acquires the mutex lock() again once
	    /// awaken after another thread called signal
	void wait();

	    /// awakes a single thread suspended after having called wait()
	    ///
	    /// \note signal() must be called between lock() and unlock(). This is only at
	    /// the time unlock() is called that another thread exits from the suspended
	    /// state.
	void signal();

	    /// awakes all threads suspended after having called wait()
	    ///
	    /// \note broadcast() must be called between lock() and unlock(). This is only at
	    /// the time unlock() is called that another thread exits from the suspended
	    /// state.
	void broadcast();

    private:
	pthread_cond_t cond;

    };

} // end of namespace

#endif

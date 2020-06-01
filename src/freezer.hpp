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

#ifndef LIBTHREADAR_FREEZER_HPP
#define LIBTHREADAR_FREEZER_HPP

    /// \file freezer.hpp
    /// \brief defines the freezer class
    ///

#include "config.h"

    // C system headers
extern "C"
{

}
    // C++ standard headers
#include <string>


    // libthreadar headers
#include "condition.hpp"

namespace libthreadar
{

	/// Class freezer is a semaphore like construct that has no maximum value

	/// initialized with a value of zero, any call to lock() reduces the value by one, any unlock() increases by one
	/// a call lock() when the current value is zero or less suspends the calling thread. A call to unlock() when
	/// at least one thread is suspended on the freezer awakes exactly one thread. A call to unlock() when the freezer
	/// value is zero or more does only increase the value the calling thread is not blocked.
    class freezer
    {
    public:
	    /// freezer constuctor
	freezer();

	    /// no copy constructor
	freezer(const freezer & ref) = delete;

	    /// no move constructor
	freezer(freezer && ref) noexcept = default;

	    /// no assignment operator (made private)
	freezer & operator = (const freezer & ref) = delete;

	    /// no move operator
	freezer & operator = (freezer && ref) noexcept = default;

	    /// Destructor
	~freezer();

	    /// Return whether the freezer has at least a pending thread waiting for another thread to unlock it
	bool waiting_thread() const;


	    /// Request a "resource"

	    /// if the freezer value is zero or less, reduces the freezer by one and suspends the calling thread
	    /// if the freezer value is strictly greater than zero, it only reduces the freezer value by one
	void lock();

	    /// Release a "resource"

	    /// increase the freezer value by one. If at least one thread was suspended on the freezer exactly one thread
	    /// is awaken
	void unlock();

	    /// Reset to initial state releasing any thread that could wait on the freezer
	void reset();

	    /// Return the value of the freezer, that's to say the number of available "resources".

	    /// \note a negative value gives the total number of threads that are suspended by calling lock() (well the opposit value of
	    /// the number of threads more precisely).
	int get_value() const { return value; };

    private:
	int value;            //< this is the freezer value
	condition cond;       //< to protect access to value
    };

} // end of namespace

#endif

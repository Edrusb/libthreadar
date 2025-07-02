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

#ifndef LIBTHREADAR_CONDITION_HPP
#define LIBTHREADAR_CONDITION_HPP

    /// \file condition.hpp
    /// \brief defines the condition class

#include "mutex.hpp"
#include "exceptions.hpp"

#include <deque>

namespace libthreadar
{

	/// Wrapper around the Posix pthread_cond_t object and its associated mutex

	/// Class condition inherits from the \ref mutex class and thus provides the \ref lock(), \ref unlock()
	/// and \ref try_lock() methods. These must be used around the \ref wait(), \ref signal() or \ref broadcast()
	/// methods brought by this class, see details for each of them.

	/// \note each object share a user defined set of condition on the same mutex
	/// as defined at constructor time. wait() takes as argument the condition number
	/// and will be awaken by a signal() or broadcast() having this same number as
	/// argument

    class condition : public mutex
    {
    public:

	    /// constructor

	    /// \param[in] num number of instance to create, each instance is a separated condition
	    /// relying on the same mutex. First instance starts with index 0
	condition(unsigned int num = 1);

	    /// no copy constructor
	condition(const condition & ref) = delete;

	    /// no move constructor
	condition(condition && ref) = default;

	    /// no assignment operator
	condition & operator = (const condition & ref) = delete;

	    /// no move operator
	condition & operator = (condition && ref) noexcept = default;

	    /// destructor
	~condition();


	    /// put the calling thread on hold waiting for another thread to call signal()

	    /// \param[in] instance the instance number to have the caller waiting on
	    /// \note wait() must be called between lock() and unlock(). Once suspendend calling
	    /// \ref wait(), the current object is transparently unlocked, for other thread to be able
	    /// to use it. The mutex is re-acquires (transparently locked) once the thread exits the wait() call it was
	    /// suspended on, which occurs if another thread calls \ref signal() with the same instance
	    /// number
	void wait(unsigned int instance = 0);

	    /// awakes a single thread suspended for having called wait() on the condition given in argument

	    /// \param[in] instance the condition number to consider, only thread having called
	    /// \ref wait() with this same instance as argument are eligible to be awaken here.
	    /// \note signal() must be called between lock() and unlock(). This is only at
	    /// the time unlock() is called that another thread exits from the suspended
	    /// state.
	void signal(unsigned int instance = 0);

	    /// awakes all threads suspended for having called wait() on the condition given in argument

	    /// \param[in] instance broadcast threads that have been waiting on that instance
	    /// \note broadcast() must be called between lock() and unlock(). This is only at
	    /// the time unlock() is called that other threads can exit from their suspended
	    /// state, by returning from their \ref wait() call.
	void broadcast(unsigned int instance = 0);

	    /// return the number of thread currently waiting on that condition

	    /// \param[in] instance the condition instance number to count the waiting thread on
	unsigned int get_waiting_thread_count(unsigned int instance = 0) const { return counter[instance]; };

    private:
	std::deque<pthread_cond_t> cond;
	std::deque<unsigned int> counter;

    };

    	/// \example ../doc/examples/condition_example.cpp

} // end of namespace

#endif

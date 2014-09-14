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

#ifndef LIBTHREADAR_THREAD_HPP
#define LIBTHREADAR_THREAD_HPP

    /// \file thread.hpp
    /// \brief holds the definition of the thread class
    ///
    /// this is inspired from http://blog.emptycrate.com/node/270 But with the difference
    /// that thread managment is done in its own pure virtual class and arbitrary
    /// threaded work is done in inherited classes. Also has been added exception transmission
    /// support from sub threaded back to parent thread using the join() method of destructor

#include "config.h"

    // C system headers
extern "C"
{
#if HAVE_PTHREAD_H
#include <pthread.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
}
    // C++ standard headers


    // libthreadar headers
#include "mutex.hpp"

namespace libthreadar
{

	/// class thread is a pure virtual class, that implements thread creation and actions
	///
	/// at the difference if the C++11 thread directive, the creation of an inherited class
	/// object does not immediately create a thread. The inherited class can provide any
	/// constructor it will as well as any other methods and fields that will be accessed
	/// by the thread method. This is far more easy to pass arguments to a thread that way.
	///
	/// inherited classes only must define the inherited_run() method, method that will be
	/// run in its specific thread.
	//
	/// Once dat has been setup, the thread can be run calling the run() method
	///
	/// the join() method can be used by the run() caller to wait for thread terminasion
	/// join() may throw any exception that could have been generated in from inherited_run()
	/// method. Object destruction kills the thread and may also generate an exception
	/// if one has been thrown from inherited_run(). For that reason it is advised to always
	/// call join() before destructor get called to avoid having to manage exception at
	/// destuction time.
	///
	/// once the thread is no more running a new call to run() is allowed to run again the
	/// thread for that object.
    class thread
    {
    public:
	    /// constructor
	thread();

	    /// destructor
	virtual ~thread();

	    /// set signal mask for this object's when the thread will be run
	void set_signal_mask(const sigset_t & mask) { sigmask = mask; };

	    /// launch the current object routing in a separated thread
	void run();

	    /// checks whether a separated thread is running the inherited_run() method of this object
	bool is_running() const { return running; };

	    /// the caller will be suspended until the current object's thread ends
	void join() const;

	    /// the caller send a cancellation request to this object's running thread if any
	void kill() const;

    protected:

	    /// action to be performed in the sperated thread
	virtual void inherited_run() = 0;

	    /// available for inherited class to avoid thread cancellation to occur in a critical section
	void suspend_cancellation_requests() const;
	    /// available for inherited class to avoid thread cancellation to occur in a critical section
	void resume_cancellation_requests() const;

    private:
	mutex field_control;           //< mutex protecting access to object's data
	bool running;                  //< whether a thread is running
	pthread_t tid;                 //< the thread ID of the running thread if any
	bool joignable;                //< whether exist status of thread has to be retrieved
	unsigned int cancellable;      //< this field is not protected by mutex as it ougth to be modified only by the spawn thread. It allows suspend/resume cancellation requests to be re-entrant (0 = cancellation accepted)
	sigset_t sigmask;              //< signal mask to use for the thread

	    /// checks whether the object is running in a separated thread
	    ///
	    /// \param[out] id returns the thread_id upon success
	    /// \return true if the object is running under a separated thread
	    /// if false is returned, the argument is not set
	bool is_running(pthread_t & id) const;


	    // static members

	static void *run_obj(void *obj);  //< called by pthread_create to spawn a new thread
	static void primitive_suspend_cancellation_requests();
	static void primitive_resume_cancellation_requests();
    };

} // end of namespace

#endif

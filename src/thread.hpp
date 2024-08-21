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

	/// Class thread is a pure virtual class, that implements thread creation and operations

	/// At the difference of the C++11 thread directive, the creation of an inherited class
	/// object does not immediately launch a thread. The inherited class can first provide any
	/// fields necessary to the thread execution by mean of inherited class constructor or better
	/// by using several customized method of the inherited class, which brings better readability
	/// and ease code maintenance than having a call with a lot of argument as C++11 requests it
	/// to be done.
	///
	/// Inherited classes must only define the inherited_run() method, method that will be
	/// run in its specific thread once the run() method will be called. The private field of
	/// inherited classes can be used to host variables only accessible by the running thread.
	/// Inherited class
	/// may also implement method to communicate with the running thread, here too the private
	/// (or protected) fields of the class can be used to host mutex if one is necessary to provide
	/// consistent information from one thread to the other.
	///
	/// The join() method can be used by the run() caller to wait for thread termination.
	/// If the corresponding thread has already ended, the join() method ends immediately.
	/// If the thread aborted due to an exception, this exception will be rethrown from the
	/// the thread calling join(). Last, calling join() on an object which thread has not yet
	/// started or which has ended and for which join() has already been run, does nothing:
	/// the join() method returns immediately.
	///
	/// \note Class thread object destruction kills the thread and could have generated an exception
	/// from inherited_run(), but this one will be caught and ignored by the thread::thread parent
	/// destructor. Under some context implementation (Cygwin) if a pthread has already started but
	/// has not yet reached the time it calls inherited_run() method, when object destructor is
	/// called, when the subthread calls the inherited_run() method, the system may report a SEGFAULT
	/// in particular when the fields of the object that are from the inherited class do
	/// not exist anymore. The kill() and join() present in the thread::~thread destructor, cannot
	/// prevent this as when the execution pointer reach them, all inherited class fields do no more
	/// exist.
	///
	/// IT IS THUS IMPORTANT FOR ANY INHERITED CLASS TO INVOKE kill() THEN join() IN THEIR DESTRUCTOR
	///
	/// Once the thread is no more running a new call to run() is allowed if a join() call has been issued
	/// since the thread was last run. This allows to run again a thread without having
	/// to pass again all the possibly many arguments and datastructures requested by this thread.
	///

    class thread
    {
    public:
	    /// constructor
	thread();

	    /// copy constructor and assignment operator are disabled
	thread(const thread & ref) = delete;
	thread(thread && ref) noexcept = default;
	thread & operator = (const thread & ref) = delete;
	thread & operator = (thread && ref) noexcept = default;

	    /// destructor
	virtual ~thread();

	    /// set signal mask for this object's when the thread will be run

	    /// \note see sigsetops(3) for details on manipulating signal sets
	void set_signal_mask(const sigset_t & mask) { sigmask = mask; };

	    /// launch the current object routing in a separated thread
	void run();

	    /// checks whether a separated thread is running the inherited_run() method of this object
	bool is_running() const { return running; };

	    /// checks whether the object is running in a separated thread

	    /// \param[out] id returns the thread_id upon success
	    /// \return true if the object is running under a separated thread
	    /// if false is returned, the argument is not set
	bool is_running(pthread_t & id) const;

	    /// the caller will be suspended until the current object's thread ends
	void join() const;

	    /// the caller send a cancellation request to this object's running thread if any

	    /// \note if the thread is suspended by the system reading or writing to a filedescriptor for
	    /// example, the thread survives up to the time it exits from that suspended state
	    /// \note libthreadar::thread::kill() is implemented over pthread_cancel which
	    /// under Linux does not work well inside C++ code when exception are caught. Starting
	    /// libthreadar 1.5.x and above the class will provide a routine implementing a cancellation
	    /// checkpoint that the subthread will have the ability to run to eventually trigger its
	    /// cancellation.
	void kill() const;

    protected:

	    /// action to be performed in the separated thread

	    /// \note There is no argument to provide, because this is the responsibility of the inherited class
	    /// to defined private/protected fields, methods and constructors to set their value
	    /// and define whether fields are only accessed by the spawn or the calling thread
	    /// or both and in that case which way to avoid concurrent access to such fields.
	virtual void inherited_run() = 0;

	    /// available for inherited class to avoid thread cancellation to occur in a critical section

	    /// any cancellation request received after a call to suspend_cancellation_requests() is delayed
	    /// until a call to resume_cancellation_requests() is issued
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

	    // static members

	static void *run_obj(void *obj);  //< called by pthread_create to spawn a new thread
	static void primitive_suspend_cancellation_requests();
	static void primitive_resume_cancellation_requests();
    };

    	/// \example ../doc/examples/thread_example.cpp
	/// this is an example of use of class libthreadar::thread
	///

} // end of namespace

#endif

/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2025 Denis Corbin
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
	/// and ease code maintenance than having a call with a lot of argument as C++11 thread managment
	/// requests it to be done.
	///
	/// Inherited classes must only define the inherited_run() method, method that will be
	/// run in its specific thread once the run() method will be called on that object. The private fields of
	/// inherited classes can be used to host variables only accessible by the running thread.
	/// Inherited class
	/// may also implement method to communicate with the running thread, here too the private
	/// (or protected) fields of the class can be used to host mutex if one is necessary to provide
	/// consistent information from the current thread to the other interacting with it by mean of
	/// adhoc methods of the inherited class.
	///
	/// The join() method can be used by the run() caller to wait for thread termination.
	/// If the corresponding thread has already ended, the join() method ends immediately.
	/// If the thread aborted due to an exception, this exception will be rethrown from the
	/// the thread calling join(). Last, calling join() on an object which thread has not yet
	/// started or which has ended and for which join() has already been run, does nothing:
	/// the join() method returns immediately.
	///
	/// \note the thread object destructor kills the thread if it was running, this could have
	/// generated an exception thrown from inherited_run() in another context, but here (thread class
	/// destructor), all are caught and ignored.
	/// Under some operating systems (Cygwin), if a pthread has already started but
	/// has not yet reached the time it has called the inherited_run() method when object destructor is
	/// called, then, when the subthread calls the inherited_run() method, the system may report a SEGFAULT
	/// in particular when the fields of the object that are from the inherited class do
	/// not exist anymore. The kill() and join() present in the thread::~thread destructor, cannot
	/// prevent this because when the execution pointer reaches them, all inherited class fields do no more
	/// exist anyway.
	///
	/// \par Important!
	/// IT IS THUS IMPORTANT FOR ANY INHERITED CLASS TO INVOKE cancel() [or other mechanism to stop the
	/// thread] and THEN join() IN THEIR DESTRUCTOR
	///
	/// Once the thread is no more running, a new call to run() is allowed but only if a join() call has been issued
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

	    /// reset the stack size to the system default value

	    /// this method must not be called when the current object has its
	    /// thread running.
	    /// \note this sets back the stack to the system default
	void reset_stack_size();

	    /// set the stack size to non default value

	    /// this method must not be called when the current object has its
	    /// thread running.
	    /// \param[in] val size in bytes of the stack to allocate and use.
	void set_stack_size(unsigned int val);

	    /// get the current stack size value

	    /// \return zero when a system default stack is used, else the argument
	    /// that has been provided to set_stack_size() method.
	unsigned int get_stack_size() const { return stack_size; };

	    /// set signal mask of the thread spawn by run()

	    /// this value persists accros several run()/join() executions.
	    /// \note see sigsetops(3) for details on manipulating signal sets
	virtual void set_signal_mask(const sigset_t & mask) { sigmask = mask; };

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

	    /// \deprecated WARNING! kill() is DEPRECATED since release 1.5.0 and will be removed in
	    /// the future as it does not work for some glibc implementations when using
	    /// C++ code (collides with exceptions). USE cancel() feature instead.
	    /// See documentation about cancel() this is *not* a 1:1 replacement of kill().
	    /// See also inherited_cancel(), and cancellation_checkpoint() methods.
	void kill() const;


	    /// the caller send a cancellation request to this object's running thread if any

	    /// \note the thread will effectively end if it runs its cancellation_checkpoint()
	    /// regularly in its loop(s), see below. As an alternative, the inherited class can
	    /// rely in the protected method inherited_cancel() to implement a mechanism to stop
	    /// the possibly running thread.
	void cancel();



    protected:

	    /// exception used to trigger thread cancellation

	    /// \note IMPORTANT: this exception should not be caught within inherited_run()
	class cancel_except
	{
	public:
	    cancel_except() {};
	    cancel_except(cancel_except &) = default;
	    cancel_except(cancel_except &&) noexcept = default;
	    cancel_except & operator = (cancel_except &) = default;
	    cancel_except & operator = (cancel_except &&) noexcept = default;
	    ~cancel_except() = default;
	};

	    /// action to be performed in the separated thread (implementation is expected in inherited classes)

	    /// \note There is no argument to provide, because this is the responsibility of the inherited class
	    /// to defined private/protected fields, methods and/or constructors to set their value.
	    /// It is also the responsibility of the inherited class to define if a (private or protected) field is
	    /// to be exclusively accessed
	    /// from the thread running inherited_run() or by the thread calling this objects or by both (if for
	    /// example the field is a mutex) to provide communication mean with the thread running the
	    /// inherited_run() method.
	virtual void inherited_run() = 0;

	    /// available withing the inherited_run() method to eventually trigger thread cancellation

	    /// \note since release 1.5.0 the kill() method has been renamed as cancel() and does no more
	    /// rely on pthread_cancel() which does not work well under C++ context since the NPTL implementation
	    /// used under Linux. The cancellation point are thus defined manually by cancellation_checkpoint()
	    /// which has to be called regularly in the implementation of your threads (see inherited_run()),
	    /// if you want to be able to call the cancel() method to stop the thread properly. Thread
	    /// cancellation relies on the libhtreadar::thread::cancel_except exception that should never
	    /// be caught in your code, either by cathing explicitely other exceptions or by preventing
	    /// it to be catch by a cath-all statement:
	    /** \verbatim
		void inherited_run()
		{
                    try
		    {
		        while(something)
		        {
		           ... // code to protect
		           cancellation_checkpoint();
		        }
		    }
		    catch(cancel_except &)
		    {
	                // eventually release some resource
	                // but propagate the exception!
		        throw;
		    }
		    catch(...)
		    {
			// a catch-all statement you want/need
		        // not rethrowing the exception
		    }
		}
	     \endverbatim **/
	    ///
	    /// Of course if the catch-all statement do rethrow all exceptions
	    /// nothing special is to be done as the cancel_except exception
	    /// will still be propagated upward and drive the thread cancellation to its end.
	void cancellation_checkpoint() const;

	    /// this method is called by cancel() even if the thread is not running()
	    /// to let inherited classes define alternative method to stop the thread running
	    /// inherited_run() than the cancellation_checkpoint() mechanism, or in complement to
	    /// it. Attention should be taken to the fact the caller of cancel() is not likely to
	    /// be the same thread as the one running inherited_run(), and mutex or other mechanism
	    /// may be necessary to avoid concurrent access to some fields in your inherited classes,
	    /// fields used to communicate the cancellation requested to the thread from the other thread
	    /// calling cancel().
	virtual void inherited_cancel() {};

    private:
	mutable mutex field_control;   ///< mutex protecting access to object's data
	bool running;                  ///< whether a thread is running
	pthread_t tid;                 ///< the thread ID of the running thread if any
	bool joignable;                ///< whether exist status of thread has to be retrieved
	mutable bool do_cancel;        ///< whether thread should cancel/stop
	sigset_t sigmask;              ///< signal mask to use for the thread
	unsigned int stack_size;       ///< stack size when non-default stack is used, 0 if system default stack is used
	char* stack;                   ///< allocated stack when non-default size is requested


	void clear_stack();

	    // static members

	static void *run_obj(void *obj);  //< called by pthread_create to spawn a new thread
    };

    	/// \example ../doc/examples/thread_example.cpp
	/// this is an example of use of class libthreadar::thread
	///

} // end of namespace

#endif

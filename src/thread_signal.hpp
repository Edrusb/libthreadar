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

#ifndef LIBTHREADAR_THREAD_SIGNAL_HPP
#define LIBTHREADAR_THREAD_SIGNAL_HPP

    /// \file thread_signal.hpp
    /// \brief holds the definition of a thread class type where cancellation is implemented by mean of signal
    ///

#include "config.h"

    // C system headers

    // C++ standard headers


    // libthreadar headers
#include "thread.hpp"

namespace libthreadar
{

	/// Class thread_signal provide the same interface as class thread but relies on a signal to wakeup the tread if it was pending on a system call

	/// this class derives from class libthread::thread and is used the same.
	/// However a signal has to be reserved for the whole process and thus for
	/// all threads to awake thread in case it would be pending on a system call.
	/// In such situation system call return EINTR code and your thread should
	/// consider this value as usually and probably retry the system call. Though,
	/// this let your code invoke the cancellation_checkpoint() method that will
	/// and only this will trigger the end of the thread by throwing an
	/// thread::cancel_except exception that should not be catched by the code
	/// of inherited_run() method. The signal handle associated to this signal
	/// does nothing, so if your code does not call cancellation_checkpoint() it will
	/// continue to run transparently (except for system call that return EINTR in
	/// that context).
	/// \note the signal used can be set using the static method change_default_signal()

    class thread_signal: public thread
    {
    public:
	    /// constructor
	thread_signal();

	    /// copy constructor and assignment operator are disabled from libhtreadar::thread
	thread_signal(const thread_signal & ref) = delete;
	thread_signal(thread_signal && ref) noexcept = default;
	thread_signal & operator = (const thread_signal & ref) = delete;
	thread_signal & operator = (thread_signal && ref) noexcept = default;

	    /// destructor
	virtual ~thread_signal() = default; // no thread to cancel nor data to cleanup here

	    /// set signal mask for this object's when the thread_signal will be run

	    /// \note see sigsetops(3) for details on manipulating signal sets

	    /// \note this is a modified version of thread::set_signal_mask() that
	    /// removes from the sigset_t the signal used to awake threads so no thread_signal
	    //  will ignore it
	void set_signal_mask(const sigset_t & mask);

	    /// change the signal used to awake threads

	    /// \note by default the signal used to awake a thread is SIGUSR2
	static void change_default_signal(int sig);

    protected:

	    /// replaces thread::inherited_cancel()

	    /// \note except the method name that changes, the purpose is the same
	virtual void signaled_inherited_cancel() {};

    private:

	    /// inherited from thread , made private
	    /// \note but this does not seem to prevent inherited
	    /// classes to override the method thread::inherited_cancel() ...
	    /// Compilation succeeded having an thread_signal derived
	    /// class overriding inherited_cancel()
	    /// instead of signaled_inherited_cancel() using g++ 10.2.1
	virtual void inherited_cancel() override;

	static mutex verrou;
	static bool initialized;
	static int awaking_signal;

	static void set_signal_handler();
	static void handler(int sig);
    };

    	/// \example ../doc/examples/thread_signal_example.cpp
	/// this is an example of use of class libthread_signalar::thread_signal
	///

} // end of namespace

#endif

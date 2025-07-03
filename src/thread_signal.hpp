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

	/// Class thread_signal provides the same interface as class \ref thread but in addition relies on a signal to awake the tread if it was pending on a system call

	/// This class derives from class libthreadar::thread and is used the same.
	/// However a signal has to be reserved for the whole process for
	/// any thread to be awaken in case it would be pending on a system call.
	/// In such situation a system call returns EINTR code and your thread implementation should
	/// consider this value as usually and probably retry the system call. Though,
	/// before retrying, this let your code invoke the cancellation_checkpoint() method
	/// to check whether an thread cancellation was requested, which method will in that case throw a
	/// thread::cancel_except exception that should not be catched by the code
	/// of inherited_run() method. The signal handle associated to this signal is implemented
	/// in this class but does nothing. So if your code does not call cancellation_checkpoint() it will
	/// continue to run normally, except for the point when the thread was pending on a system call
	/// which will in that case return EINTR.
	/// \note the signal used for this class can be set using the static method change_default_signal()

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
	    ///
	    /// \note this is a modified version of thread::set_signal_mask() that
	    /// removes from the sigset_t the signal used to awake threads so no thread_signal
	    ///  will ignore it
	virtual void set_signal_mask(const sigset_t & mask) override;

	    /// change the signal used to awake threads

	    /// \note by default the signal used to awake a thread is SIGUSR2

	    /// \note your program should not define any handle for that signal, this
	    /// is the class, globally, that will define it at the time the first object
	    /// of an inherited class will be constructed or at the next time an object
	    /// is constructed if change_default_signal() is used to change the signal
	    /// to use after an object has already been created.
	    /// This call has thus no effect until a new object from an inherited class is created.
	static void change_default_signal(int sig);

    protected:

	    /// replaces thread::inherited_cancel() and should be used instead of it

	    /// \note except the method name that changes, the purpose is the same as inherited_cancel():
	    /// you have the possibility to provide your own implementation of signaled_inherited_cancel()
	    /// to implement an alternative method of cancelling a thread. This method will be invoked when
	    /// the thread::cancel() method will be called, right before the signal is sent to the thread if it
	    /// was running.
	virtual void signaled_inherited_cancel() {};

    private:

	    /// inherited from thread, made private
	    /// \note but this does not seem to prevent inherited
	    /// classes to override the method thread::inherited_cancel() ...
	    /// Compilation succeeded having an thread_signal derived
	    /// class overriding inherited_cancel()
	    /// instead of signaled_inherited_cancel() using g++ 10.2.1
	virtual void inherited_cancel() override;

	    /// send the awaking signal to the thread
	void send_signal();

	static mutex verrou;
	static bool initialized;
	static int awaking_signal;

	static void set_signal_handler();
	static void handler(int sig);
    };

} // end of namespace

#endif

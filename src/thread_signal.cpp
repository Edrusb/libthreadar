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

#include "config.h"

    // C system headers
extern "C"
{
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_ERRNO_H
#include <errno.h>
#endif
}
    // C++ standard headers


    // libthreadar headers
#include "exceptions.hpp"

    // this module's header
#include "thread_signal.hpp"


using namespace std;

namespace libthreadar
{
    mutex thread_signal::verrou;
    bool thread_signal::initialized = false;
    int thread_signal::awaking_signal = SIGUSR2;

    thread_signal::thread_signal()
    {
	set_signal_handler();
    }


    void thread_signal::set_signal_mask(const sigset_t & mask)
    {
	if(sigismember(&mask, awaking_signal))
	    throw exception_range("requested to mask the signal used by thread_signal class");
	thread::set_signal_mask(mask);
    }

    void thread_signal::change_default_signal(int sig)
    {
	verrou.lock();
	try
	{
	    if(awaking_signal == SIGCHLD
	       || awaking_signal == SIGKILL)
		throw exception_range("forbidden signal given for libthreadar::thread_signal::change_default_signal()");
	    initialized = false;
	    awaking_signal = sig;
	}
	catch(...)
	{
	    verrou.unlock();
	    throw;
	}
	verrou.unlock();

	set_signal_handler();
    }

    void thread_signal::inherited_cancel()
    {
	try
	{
		// preparing the thread to report cancellation request status
		// (if implemented in inherited class)
	    signaled_inherited_cancel();
	    send_signal();
	}
	catch(thread::cancel_except & e)
	{
	    throw;
	}
	catch(...)
	{
	    send_signal();
	    throw;
	}
    }

    void thread_signal::send_signal()
    {
	pthread_t tid;

	    // awaking the thread if it was pending on a system call
	if(is_running(tid))
	{
	    if(pthread_kill(tid, awaking_signal) != 0)
		throw exception_system("Error calling pthread_kill(): ", errno);
	}
    }


    void thread_signal::set_signal_handler()
    {
	if(!initialized) // reading only without acquiring the lock for efficiency
	{
	    verrou.lock();
	    try
	    {
		    // we could have been pending on verrou
		    // while another thread would have performed
		    // the initialization
		if(!initialized)
		{
		    struct sigaction sigac;

		    sigac.sa_handler = handler;
		    if(sigemptyset(& sigac.sa_mask) != 0)
			throw exception_system("Error calling sigemptyset(): ", errno);
		    sigac.sa_flags = 0;

		    if(sigaction(awaking_signal, &sigac, nullptr) != 0)
			throw exception_system("Error calling sigaction(): ", errno);
		    initialized = true;
		}
	    }
	    catch(...)
	    {
		verrou.unlock();
		throw;
	    }
	    verrou.unlock();
	}
    }


    void thread_signal::handler(int sig)
    {
	    // we do nothing!
	    // the handler by itself is only
	    // need to have possibly blocking system call
	    // to exit, for the thread have a chance to
	    // call cancellation_checkpoint()
	    // this is the duty of the inherited_run()
	    // implementation to not have several system
	    // call possibly blocking before reaching a
	    // cancellation_checkpoint()
    }


} // end of namespace

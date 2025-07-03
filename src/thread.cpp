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

#include "config.h"

    // C system headers
extern "C"
{
#if HAVE_ERRNO_H
#include <errno.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#endif
}
    // C++ standard headers


    // libthreadar headers
#include "exceptions.hpp"
#include "tools.hpp"

    // this module's header
#include "thread.hpp"


using namespace std;

namespace libthreadar
{

    thread::thread()
    {
	running = false;
	joignable = false;
	do_cancel = false;
	stack_size = 0;
	stack = nullptr;
	sigemptyset(&sigmask);
    }

    thread::~thread()
    {
	try
	{
	    cancel();
	    join();
	}
	catch(...)
	{
		// a destructor should not generate execptions
		// such exceptions should have been generated earlier using join() if
		// it had any importance to be taken care of
	}
	clear_stack();
    }

    void thread::reset_stack_size()
    {
	field_control.lock();
	try
	{
	    if(running)
		throw exception_thread("Cannot change stack size while the thread is running");

	    clear_stack();
	    stack_size = 0;
	}
	catch(...)
	{
	    field_control.unlock();
	    throw;
	}
	field_control.unlock();
    }

    void thread::set_stack_size(unsigned int val)
    {
	field_control.lock();
	try
	{
	    if(running)
		throw exception_thread("Cannot change stack size while the thread is running");

	    clear_stack();
	    stack = new (nothrow) char[val];
	    if(stack == nullptr)
	    {
		stack_size = 0;
		throw exception_memory();
	    }
	    else
		stack_size = val;
	}
	catch(...)
	{
	    field_control.unlock();
	    throw;
	}
	field_control.unlock();
    }


    void thread::run()
    {
	pthread_attr_t thread_attribs;

	switch(pthread_attr_init(&thread_attribs))
	{
	case 0:
	    break;
	case ENOMEM:
	    throw exception_memory();
	default:
	    throw THREADAR_BUG;
	}

	     // critical section for thread creation

	field_control.lock();
	try
	{
		// sanity checks

	    if(running)
		throw exception_thread("Cannot run thread, object already running in a sperated thread");
	    if(joignable)
		throw exception_thread("Previous thread has not been joined and possibly returned exception is deleted");


		// attribute settings

	    if(stack_size != 0)
	    {
		if(stack == nullptr)
		    throw THREADAR_BUG;

		switch(pthread_attr_setstack(&thread_attribs,
					     stack,
					     stack_size))
		{
		case 0:
		    break;
		case EINVAL:
		    throw exception_range("Stack size requested too small");
		case EACCES:
		    throw THREADAR_BUG;
			// man page for pthread_attr_set_stack mention this possible error
			// when the provided stack is not both readable and writable by the
			// caller, here this should always be the case
		default:
		    throw THREADAR_BUG;
		}
	    }


		// thread creation

	    do_cancel = false;
	    if(pthread_create(&tid, &thread_attribs, run_obj, this) != 0)
		throw exception_system("Failed creating a new thread: ", errno);
	    running = true;
	    joignable = true;
	}
	catch(...)
	{
	    field_control.unlock();
	    (void)pthread_attr_destroy(&thread_attribs);
	    throw;
	}
	field_control.unlock();
	(void)pthread_attr_destroy(&thread_attribs);
    }

    bool thread::is_running(pthread_t & id) const
    {
	bool ret;
	mutex *mut = const_cast<mutex *>(&field_control);

	if(is_running())
	{
	    mut->lock();

	    ret = running;
	    if(running)
		id = tid;

	    mut->unlock();

	    return ret;
	}
	else
	    return false;
    }

    void thread::join() const
    {
	pthread_t dyn_tid;
	thread *me = const_cast<thread *>(this);

	if(joignable)
	{
	    void *returned_exception;
	    int ret = pthread_join(tid, &returned_exception);

	    me->joignable = false;
	    if(ret != ESRCH && ret != 0)
	    {
		if(errno != 0)
		    throw exception_system("Failed joining thread: ", errno);
		else
		{
		    switch(ret)
		    {
		    case EDEADLK:
			throw exception_thread("Deadlock was detected");
		    case EINVAL:
			throw exception_thread("Tried to join a non-joinable thread or another thread is already waiting to join this same target thread");
		    case ESRCH:
			throw THREADAR_BUG; // this has condition has already been treated above
		    default:
			throw exception_thread(string("pthread_join() system called returned an unknown error: ") + tools_convert_to_string(ret));
		    }
		}
	    }

	    if(returned_exception != nullptr && returned_exception != PTHREAD_CANCELED)
	    {
		exception_ptr *ebase = reinterpret_cast<exception_ptr *>(returned_exception);
		if(ebase == nullptr)
		    throw THREADAR_BUG;
		try
		{
		    rethrow_exception(*ebase);
		}
		catch(...)
		{
		    delete ebase;
		    throw;
		}
		throw THREADAR_BUG; // we should never leave without throwing an exception thus this statment should never be reached
	    }
	}
    }

    void thread::kill() const
    {
	pthread_t dyn_tid;

	if(is_running(dyn_tid))
	{
	    thread *me = const_cast<thread *>(this);
	    int ret;

	    ret = pthread_cancel(dyn_tid);
	    if(ret != ESRCH && ret != 0)
		throw exception_system("Failed killing thread: ", errno);

	    if(me == nullptr)
		throw THREADAR_BUG;
	    me->running = false;
	}
    }

    void thread::cancel()
    {
	field_control.lock();
	try
	{
	    do_cancel = true;
	}
	catch(...)
	{
	    field_control.unlock();
	    throw;
	}
	field_control.unlock();

	inherited_cancel();
    }

    void thread::cancellation_checkpoint() const
    {
	    // we run without lock the field_control mutex
	    // for efficiency
	    // either do_cancel value is still true
	    // either is not but it cannot have other
	    // values so locking is not necessary to read
	    // this value
	if(do_cancel)
	    throw cancel_except();
    }

    void thread::clear_stack()
    {
	if(stack != nullptr)
	{
	    delete [] stack;
	    stack = nullptr;
	}
    }

    void *thread::run_obj(void *obj)
    {
	exception_ptr *ret = nullptr;

	try
	{
	    thread *tobj = reinterpret_cast<thread *>(obj);
	    if(tobj == nullptr)
		throw THREADAR_BUG;

		// locking and unlocking object's mutex is a simple form of barrier
		// this way we start working only when the caller has exited run()
	    tobj->field_control.lock();
	    tobj->field_control.unlock();
	    if(pthread_sigmask(SIG_SETMASK , &(tobj->sigmask), NULL) != 0)
		throw exception_system("Failing setting signal mask for thread", errno);

	    try
	    {
		tobj->inherited_run();
	    }
	    catch(cancel_except &)
	    {
		    // nothing to do
		    // this exception
		    // must not been
		    // rethrown when another
		    // thread will call join()
	    }
	    catch(...)
	    {
		    // no need to use the mutex here as only this thread has to set
		    // back running to false
		tobj->running = false;
		throw;
	    }
	    tobj->running = false;
	}
	catch(...)
	{
	    ret = new (nothrow) exception_ptr(current_exception());
	}

	return ret;
    }

} // end of namespace

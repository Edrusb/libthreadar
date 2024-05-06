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
    }

    void thread::run()
    {
	field_control.lock();

	try
	{
	    if(running)
		throw exception_thread("Cannot run thread, object already running in a sperated thread");
	    if(joignable)
		throw exception_thread("Previous thread has not been joined and possibly returned exception is deleted");
	    do_cancel = false;
	    if(pthread_create(&tid, NULL, run_obj, this) != 0)
		throw exception_system("Failed creating a new thread: ", errno);
	    running = true;
	    joignable = true;
	}
	catch(...)
	{
	    field_control.unlock();
	    throw;
	}
	field_control.unlock();
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
		throw exception_system("Failed joining thread: ", errno);
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

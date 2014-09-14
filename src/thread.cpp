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
	cancellable = 0; // is cancellable at startup
	sigemptyset(&sigmask);
    }

    thread::~thread()
    {
	try
	{
	    kill();
	    join();
	}
	catch(exception_bug & e)
	{
	    throw;
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
	thread::primitive_suspend_cancellation_requests();
	try
	{
	    field_control.lock();

	    try
	    {
		if(running)
		    throw exception_thread("Cannot run thread, object already running in a sperated thread");
		if(joignable)
		    throw exception_thread("Previous thread has not been joined and possibly returned exception is deleted");
		cancellable = 0; // should not be needed, but does not hurt
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
	catch(...)
	{
	    thread::primitive_resume_cancellation_requests();
	    throw;
	}
	thread::primitive_resume_cancellation_requests();
    }

    bool thread::is_running(pthread_t & id) const
    {
	bool ret;
	mutex *mut = const_cast<mutex *>(&field_control);

	if(is_running())
	{
	    thread::primitive_suspend_cancellation_requests();
	    try
	    {
		mut->lock();

		ret = running;
		if(running)
		    id = tid;

		mut->unlock();
	    }
	    catch(...)
	    {
		thread::primitive_resume_cancellation_requests();
		throw;
	    }
	    thread::primitive_resume_cancellation_requests();

	    return ret;
	}
	else
	    return false;
    }

    void thread::join() const
    {
	pthread_t dyn_tid;
	thread *me = const_cast<thread *>(this);

	if(joignable || is_running())
	{
	    void *returned_exception;
	    int ret = pthread_join(tid, &returned_exception);

	    me->joignable = false;
	    if(ret != ESRCH && ret != 0)
		throw exception_system("Failed joining thread: ", errno);
	    if(returned_exception != NULL && returned_exception != PTHREAD_CANCELED)
	    {
		exception_ptr *ebase = reinterpret_cast<exception_ptr *>(returned_exception);
		if(ebase == NULL)
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

	    if(me == NULL)
		throw THREADAR_BUG;
	    me->running = false;
	}
    }

    void thread::suspend_cancellation_requests() const
    {
	if(cancellable == 0)
	    thread::primitive_suspend_cancellation_requests();
	++(*(const_cast<unsigned int *>(&cancellable)));
    }

    void thread::resume_cancellation_requests() const
    {
	if(cancellable == 0)
	    throw THREADAR_BUG;
	--(*(const_cast<unsigned int *>(&cancellable)));
	if(cancellable == 0)
	    thread::primitive_resume_cancellation_requests();
    }


    void *thread::run_obj(void *obj)
    {
	exception_ptr *ret = NULL;

	try
	{
	    thread *tobj = reinterpret_cast<thread *>(obj);
	    if(tobj == NULL)
		throw THREADAR_BUG;

		// locking and unlocking object's mutex is a simple form of barrier
		// this way we start working only when the caller has exited run()
	    thread::primitive_suspend_cancellation_requests();
	    tobj->field_control.lock();
	    tobj->field_control.unlock();
	    if(pthread_sigmask(SIG_SETMASK , &(tobj->sigmask), NULL) != 0)
		throw exception_system("Failing setting signal mask for thread", errno);
		//
	    thread::primitive_resume_cancellation_requests();

	    try
	    {
		tobj->inherited_run();
	    }
	    catch(...)
	    {
		    // no need to use the mutex here as only this thread has to set
		    // back running to false
		tobj->running = false;
		throw;
	    }
	    tobj->running = false;

		// no need to use mutex here neither as the thread is ending and
		// whatever another thread reads (true or false) both will work
		// as no exception has to be catched and delete.
	    tobj->joignable = false;
	}
	catch(...)
	{
	    ret = new (nothrow) exception_ptr(current_exception());
	}

	return ret;
    }

    void thread::primitive_suspend_cancellation_requests()
    {
	int previous_state;

	if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &previous_state) != 0)
	    throw exception_thread("unable to set cancellation state to disable");
    }

    void thread::primitive_resume_cancellation_requests()
    {
	int previous_state;

	if(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &previous_state) != 0)
	    throw exception_thread("unable to set cancellation state to disable");
    }

} // end of namespace

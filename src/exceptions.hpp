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

#ifndef LIBTHREADAR_EXCEPTIONS_HPP
#define LIBTHREADAR_EXCEPTIONS_HPP

    /// \file exceptions.hpp
    /// \brief defines a set of exceptions that are used by libthreadar
    ///
    //. exception_base is a pure virtual class parent of all libthreadar exceptions
    //. exception_memory is used to report failure due to lack of memory
    //. exception_bug is used to signal a bug in libthreadar
    //. exception_thread is used to signal an error relative to threads (thread creation failure, etc.)
    //. exception_system is used to report a system error
    //. exception_range is used to signal out of range parameter
    //. exception_feature is used to signal an non yet implemented feature
    ///
    /// over their type, exception contains a list of message that start from the source of the error
    /// and continue by the context it occured larger and larger.
    /// the number of such message is given by size() and messages are accessible by the operator []
    /// of the exception object.

#include "config.h"

    // C system headers
extern "C"
{
#if HAVE_STRING_H
#include <string.h>
#endif
}
    // C++ standard headers
#include <string>
#include <vector>
#include <new>
#include <iostream>


    // libthreadar headers



namespace libthreadar
{

    /// pure virtual class parent of all webdar exceptions
    class exception_base
    {
    public:
	exception_base(const std::string & x_msg) { msg_table.push_back(x_msg); };

	virtual ~exception_base() {};

	    /// to be used in a catch clause to add context information before rethrowing the exception
	void push_message(const std::string & x_msg) { msg_table.push_back(x_msg); };

	    /// for site which need to display the information to the user, get the size of the message table
	unsigned int size() const { return msg_table.size(); };
	    /// get the content of the message table
	const std::string & operator [](unsigned int i) const { return msg_table[i]; };

	virtual exception_base *clone() const = 0;

    protected:
	void reset_first_message(const std::string & msg) { msg_table[0] = msg; };

    private:
	std::vector<std::string> msg_table;
    };

    class exception_memory;

    template<class T> exception_base *cloner(void * const ptr) { exception_base *ret = new (std::nothrow) T(*(reinterpret_cast<T const *>(ptr))); if(ret == NULL) throw exception_memory(); return ret; };

	///  exception used to report memory allocation failures

    class exception_memory : public exception_base
    {
    public:
	exception_memory() : exception_base("lack of memory") {};

    protected:
	virtual exception_base *clone() const { return cloner<exception_memory>((void *)this); };
    };

	/// exception used to report webdar internal bugs

#define THREADAR_BUG exception_bug(__FILE__, __LINE__)

    class exception_bug : public exception_base
    {
    public:
	exception_bug(const std::string & file, int line) : exception_base(std::string("WEBDAR: BUG MET IN File " + file + " line " + std::to_string(line))) {};

    protected:
	virtual exception_base *clone() const { return cloner<exception_bug>((void *)this); };
    };


	/// exception used to report error met when manipulating threads

    class exception_thread : public exception_base
    {
    public:
	exception_thread(const std::string & x_msg) : exception_base(x_msg) {};

    protected:
	virtual exception_base *clone() const { return cloner<exception_thread>((void *)this); };
    };

	/// exception used to report operating system errors

    class exception_system : public exception_base
    {
    public:
	exception_system(const std::string & context, int error_code);

    protected:
	virtual exception_base *clone() const { return cloner<exception_system>((void *)this); };
    };

	/// exception used to report out or range value or argument

    class exception_range : public exception_base
    {
    public:
	exception_range(const std::string & msg): exception_base(msg) {};

    protected:
	virtual exception_base *clone() const { return cloner<exception_range>((void *)this); };
    };


	/// exception used to report an non-implemented feature

    class exception_feature : public exception_base
    {
    public:
	exception_feature(const std::string & feature_name): exception_base(std::string("Unimplemented feature: ") + feature_name) {};

    protected:
	virtual exception_base *clone() const { return cloner<exception_feature>((void *)this); };
    };

} // end of namespace

#endif

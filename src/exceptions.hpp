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
    /// \brief defines a set of exceptions that are used by libthreadar to report error situations
    ///
    /// - exception_base is a pure virtual class parent of all libthreadar exceptions
    /// - exception_memory is used to report failure due to lack of memory
    /// - exception_bug is used to signal a bug in libthreadar
    /// - exception_thread is used to signal an error relative to threads (thread creation failure, etc.)
    /// - exception_system is used to report a system error (other than thread relative one)
    /// - exception_range is used to signal out of range parameter
    /// - exception_feature is used to signal a non yet implemented feature
    ///
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
#include <sstream>

    // libthreadar headers

namespace libthreadar
{

    /// Pure virtual class parent of all webdar exceptions

    /// Over the different classes inherited from exception_base, all libthreadar exceptions contain a list of
    /// message that start from the source of the error
    /// followed by messages added by each try/catch context it occured in.
    /// You can access these messages by two ways:
    /// - Either one by one, with size of the table given by the size() method and message content accessible by the operator []
    /// of the exception object. Index zero in that table gives the source of the error, greater indexes
    /// provide information on the context the error occured in.
    /// - Or by calling get_message() method to obtain a single std::string variable resulting of the concatenation of the list of messages.
    ///
    /// You may ignore other features relative to exceptions used by libthreadar to report error while still able to completely use libthreadar.
    ///
    /// However you are allowed to add new messages to the stack using push_message()
    /// and then rethrow the exception for propagation.
    ///
    /// Why not concatenating string at each catch clause and propagating the exception?
    /// because depending on languagues some may prefer to present this nested informations
    /// another way than separating them with ':' from the less specific to the root cause
    ///
    class exception_base
    {
    public:
	    /// constructor

	    /// \note used inside libthreadar only
	exception_base(const std::string & x_msg) { msg_table.push_back(x_msg); };

	    /// destructor
	virtual ~exception_base() {};

	    /// to be used in a catch clause to add context information before rethrowing the exception
	void push_message(const std::string & x_msg) { msg_table.push_back(x_msg); };

	    /// for site which need to display the information to the user

	    /// \return the size of the message list
	unsigned int size() const { return msg_table.size(); };

	    /// for site which need to display the information to the user

	    /// \param[in] i is the index of the element of the list to return, it must be stricly lesser than size()
	    /// \return the requested element of the message list
	const std::string & operator [](unsigned int i) const { return msg_table[i]; };

	    /// concatenated messages and use the given separator between messages

	    /// \param[in] sep is a string to insert between messages of the list (like for example ": ")
	    /// \return the resulting error message of the concatenation
	std::string get_message(const std::string & sep) const;

	    /// create a new object of the same type and value of the object which clone() method is invoked

	    /// \note this is true for pointer to inherited class, even if the pointer is of type exception_base
	    /// the pointed to object will be of the same type of the inherited class
	virtual exception_base *clone() const = 0;

    protected:
	    /// for libthreader internal use only
	void reset_first_message(const std::string & msg) { msg_table[0] = msg; };

    private:
	std::vector<std::string> msg_table;
    };

	/// Template used by libthreadar to implement the clone() method for libthreadar exceptions

    template<class T> exception_base *cloner(void * const ptr);

	/// Exception used to report memory allocation failures

	/// see exception_base for usage
    class exception_memory : public exception_base
    {
    public:
	exception_memory() : exception_base("lack of memory") {};

    protected:
	virtual exception_base *clone() const { return cloner<exception_memory>((void *)this); };
    };

    template<class T> exception_base *cloner(void * const ptr) { exception_base *ret = new (std::nothrow) T(*(reinterpret_cast<T const *>(ptr))); if(ret == NULL) throw exception_memory(); return ret; };


	/// Macro used to throw an exception_bug when execution reach that statement

#define THREADAR_BUG exception_bug(__FILE__, __LINE__)

	/// Exception used to report webdar internal bugs

	/// see exception_base for usage
    class exception_bug : public exception_base
    {
    public:
	exception_bug(const std::string & file, int line) : exception_base("LIBTHREADAR BUG MET IN File " + file + " line " + std::to_string(line)) {};

    protected:
	virtual exception_base *clone() const { return cloner<exception_bug>((void *)this); };
    };


	/// Exception used to report error met when manipulating threads

	/// see exception_base for usage
    class exception_thread : public exception_base
    {
    public:
	exception_thread(const std::string & x_msg) : exception_base(x_msg) {};

    protected:
	virtual exception_base *clone() const { return cloner<exception_thread>((void *)this); };
    };

	/// Exception used to report operating system errors

	/// see exception_base for usage
    class exception_system : public exception_base
    {
    public:
	exception_system(const std::string & context, int error_code);

    protected:
	virtual exception_base *clone() const { return cloner<exception_system>((void *)this); };
    };

	/// Exception used to report out or range value or argument

	/// see exception_base for usage
    class exception_range : public exception_base
    {
    public:
	exception_range(const std::string & msg): exception_base(msg) {};

    protected:
	virtual exception_base *clone() const { return cloner<exception_range>((void *)this); };
    };


	/// Exception used to report an non-implemented feature

	/// see exception_base for usage
    class exception_feature : public exception_base
    {
    public:
	exception_feature(const std::string & feature_name): exception_base(std::string("Unimplemented feature: ") + feature_name) {};

    protected:
	virtual exception_base *clone() const { return cloner<exception_feature>((void *)this); };
    };

} // end of namespace

#endif

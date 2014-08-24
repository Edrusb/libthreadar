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

#ifndef LIBTHREADAR_TAMPON_H
#define LIBTHREADAR_TAMPON_H

    /// \file tampon.hpp
    /// \brief holds a datastructure that provides asynchronous pipe-like (unidirectional) communication between two threads

#include "config.h"

    // C system headers
extern "C"
{

}
    // C++ standard headers


    // libthreadar headers
#include "semaphore.hpp"
#include "exceptions.hpp"

namespace libthreadar
{

	///  class tampon provides asynchronous communication between two threads
	///
	/// a first thread is defined as a feeder and feeds the object with data
	/// that the other thread known as the fetcher will read at a later time.
	/// If the tampon is empty the fetch thread is suspended, if it is full
	/// this is the feeder thread instead that is suspended. Feeding an empty
	/// tampon automatically awakes the fetcher thread if it was suspended for reading,
	/// reading a full tempon automatically awakes a feeder that was suspended
	/// on that object.
	///
	/// The feeder has to proceed in two steps:
	//. first call get_block_to_feed() and assign data to that block not exceeding
	//  the number "num" of entry of type T
	//. once the block of data is written to, call feed() with that block.
	/// The feeder must not feed() the tampon with any other block than the one
	/// obtained by get_block_to_feed(). The obtained blocks remains the property
	/// of the tampon object and will be released by it.
	/// Only one block can be obtained at a time, thus get_block_to_feed() and feed()
	/// should be called alternatively.
	///
	/// The fetcher interface is almost symmetric, and also has two steps too:
	//. first fetch() a new block of data, read the data from the obtained block of
	/// data of given size 'num'.
	//. then give back the block to the tampon object calling fetch_recycle().
	/// the Fetcher has not to delete the fetched block not it must fetch_recycle()
	/// another block than the one that has been fetched.
	/// only one block can be fetched at a time, thus fetch() and fetch_recycle() should
	/// be called alternatively.
	///
	/// Only on thread can be a feeder, only one (other) thread can be a fetcher.
	///
	/// tampon objects cannot be copied, once created they can only be passed as reference or
	/// using a pointer to them.

    template <class T> class tampon
    {
    public:
	    /// constructor
	    ///
	    /// \param[in] max_block is the maximum number of buffers that can be written to without being read
	    /// \param[in] block_size is the maximum size of each buffer
	tampon(unsigned int max_block, unsigned int block_size);

	    /// copy constructor is disabled and generates an exception if called
	tampon(const tampon & ref): waiting_feeder(1), waiting_fetcher(1) { throw THREADAR_BUG; };

	    /// assignment operator is disabled and generates an exception if called
	const tampon & operator = (const tampon & ref) { throw THREADAR_BUG; };

	    /// the destructor releases all inernally allocated blocks even if they have been fetched
	    /// or obtained for feeding.
	~tampon();

	    /// feeder call step 1
	    ///
	    /// provides an area where to write down data to
	    /// note that the caller shall never release the address pointed to by ptr
	void get_block_to_feed(T * & ptr, unsigned int & num);

	    /// feeder call step 2
	    ///
	    /// once the data has been set in the previously obtained buffern, it is given back to the tampon
	    /// written is the number of element written, written shall be less than or equal to the argument "num"
	    /// returned by get_block_to_feed;
	void feed(T* ptr, unsigned int written);

	    /// put back the block obtained by get_block_to_feed() because it was not used so it will be the next returned by get_block_to_feed
	void feed_cancel_get_block(T *ptr);

	    /// fetch call step 1
	    ///
	    /// \param[out] ptr is the address of the data to be read
	    /// \param[out] num is the number of element available for reading
	    /// \note that the caller shall never release the address pointed to by ptr
	void fetch(T* & ptr, unsigned int & num);

	    /// fetcher call step 2
	    ///
	    /// once data has been read, recycle the block into the tampon object
	void fetch_recycle(T* ptr);

	    /// put back the fetched block if all some data remain unfetched for now in this block,
	    ///
	    /// \note this is the duty of the caller to modify the block for the part of the data
	    /// that has been fetch be suppressed and the unfetched data becomes at the beginning of
	    /// the block. The size is thus modified and provided as new amount of available data in that block
	void fetch_push_back(T *ptr, unsigned int new_num);

	    /// for fetcher to know whether the next feed will be blocking
	bool is_empty() const { return next_feed == next_fetch; };
	bool is_not_empty() const { return !is_empty(); };

    private:
	struct atom
	{
	    T* mem;
	    unsigned int data_size;

	    atom() { mem = NULL; data_size = 0; };
	};

	atom *table;              //< datastructure holding data in transit between two threads
	unsigned int table_size;  //< size of table, i.e. number of struct atom it holds
	unsigned int alloc_size;  //< size of allocated memory for each atom in table
	unsigned int next_feed;   //< index in table of the next atom to use for feeding table
	unsigned int next_fetch;  //< index in table of the next atom to use for fetch table
	bool fetch_outside;       //< if set to true, table's index pointed to by next_fetch is used by the fetcher
	bool feed_outside;        //< if set to true, table's index pointed to by next_feed is used by the feeder
	semaphore waiting_feeder; //< feeder thread may be stuck waiting on that semaphore if table is full
	semaphore waiting_fetcher;//< fetcher thread may be stuck waiting on that semaphore if table is empty

	bool is_full() const;
	bool is_not_full() const { return !is_full(); };
	void shift_by_one(unsigned int & x); // cyclicly shift an index (next_feed or next_fetch) by one position
    };

    template <class T> tampon<T>::tampon(unsigned int max_block, unsigned int block_size): waiting_feeder(1), waiting_fetcher(1)
    {
	table_size = max_block;
	table = new atom[table_size];
	if(table == NULL)
	    throw exception_memory();
	try
	{
	    alloc_size = block_size;
	    try
	    {
		for(unsigned int i = 0 ; i < table_size ; ++i)
		{
		    table[i].mem = new T[alloc_size];
		    if(table[i].mem == NULL)
			throw exception_memory();
		    table[i].data_size = 0;
		}
		next_feed = 0;
		next_fetch = 0;
		fetch_outside = false;
		feed_outside = false;
		waiting_feeder.lock();
		try
		{
		    waiting_fetcher.lock();
		}
		catch(...)
		{
		    waiting_feeder.unlock();
		    throw;
		}
	    }
	    catch(...)
	    {
		for(unsigned int i = 0; i < table_size ; ++i)
		{
		    if(table[i].mem != NULL)
			delete [] table[i].mem;
		}

		throw;
	    }
	}
	catch(...)
	{
	    if(table != NULL)
		delete [] table;
	    throw;
	}
    }


    template <class T> tampon<T>::~tampon()
    {
	if(table != NULL)
	{
	    for(unsigned int i = 0 ; i < table_size ; ++i)
	    {
		if(table[i].mem != NULL)
		    delete [] table[i].mem;
	    }
	    delete [] table;
	}
	waiting_feeder.unlock();
	waiting_fetcher.unlock();
    }

    template <class T> void tampon<T>::get_block_to_feed(T * & ptr, unsigned int & num)
    {
	if(feed_outside)
	    throw exception_range("feed already out!");
	if(is_full())
	    waiting_feeder.lock();
	if(is_full())
	    throw THREADAR_BUG; // still full!
	feed_outside = true;
	ptr = table[next_feed].mem;
	num = alloc_size;
    }

    template <class T> void tampon<T>::feed(T *ptr, unsigned int num)
    {
	if(!feed_outside)
	    throw exception_range("fetch not outside!");
	if(ptr != table[next_feed].mem)
	    throw exception_range("returned ptr is not the one given earlier for feeding");
	feed_outside = false;
	table[next_feed].data_size = num;
	shift_by_one(next_feed);
	if(waiting_fetcher.waiting_thread())
	    waiting_fetcher.unlock();
    }

    template <class T> void tampon<T>::feed_cancel_get_block(T *ptr)
    {
	if(!feed_outside)
	    throw exception_range("fetch not outside!");
	if(ptr != table[next_feed].mem)
	    throw exception_range("returned ptr is not the one given earlier for feeding");
	feed_outside = false;
    }

    template <class T> void tampon<T>::fetch(T* & ptr, unsigned int & num)
    {
	if(fetch_outside)
	    throw exception_range("already fetched block outside");
	if(is_empty())
	    waiting_fetcher.lock();
	if(is_empty())
	    throw THREADAR_BUG;
	fetch_outside = true;
	ptr = table[next_fetch].mem;
	num = table[next_fetch].data_size;
    }

    template <class T> void tampon<T>::fetch_recycle(T* ptr)
    {
	if(!fetch_outside)
	    throw exception_range("no block outside for fetching");
	if(ptr != table[next_fetch].mem)
	    throw exception_range("returned ptr is no the one given earlier for fetching");
	fetch_outside = false;
	shift_by_one(next_fetch);
	if(waiting_feeder.waiting_thread())
	    waiting_feeder.unlock();
    }

    template <class T> void tampon<T>::fetch_push_back(T* ptr, unsigned int new_num)
    {
	if(!fetch_outside)
	    throw exception_range("no block outside for fetching");
	if(ptr != table[next_fetch].mem)
	    throw exception_range("returned ptr is not the one given earlier for fetching");
	fetch_outside = false;
	table[next_fetch].data_size = new_num;
    }

    template <class T> bool tampon<T>::is_full() const
    {
	if(next_feed + 1 < table_size)
	    if(next_fetch <= next_feed)
		return false;
	    else
		return next_fetch - next_feed > 1;
	else
	    return next_fetch > 0;
    }

    template <class T> void tampon<T>::shift_by_one(unsigned int & x)
    {
	++x;
	if(x >= table_size)
	    x = 0;
    }

} // end of namespace

#endif


/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2015 Denis Corbin
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

#ifndef LIBTHREADAR_FAST_TAMPON_H
#define LIBTHREADAR_FAST_TAMPON_H

    /// \file fast_tampon.hpp
    /// \brief defines the fast_tampon class that provides simplified but fast asynchronous pipe-like (unidirectional) communication between two threads

#include "config.h"

    // C system headers
extern "C"
{

}
    // C++ standard headers


    // libthreadar headers
#include "mutex.hpp"
#include "condition.hpp"
#include "exceptions.hpp"

namespace libthreadar
{

	///  Class fast_tampon provides asynchronous communication between two threads

    	/// A first thread is defined as a *feeder* and feeds the fast_tampon object with data
	/// that the other thread known as the *fetcher* will read at a later time.
	/// If the fast_tampon is empty the fetcher thread is suspended, if it is full
	/// the feeder thread is suspended. Feeding an empty
	/// fast_tampon automatically awakes the fetcher thread if it was suspended for reading,
	/// reading a full fast_tampon automatically awakes a feeder that was suspended
	/// for writing.
	///
	/// The feeder has to proceed in two steps:
	/// - first call get_block_to_feed() and write data to the obtained block
	/// - once the data has been written to the block, it must call feed() with that block.
	/// The feeder must not feed() the fast_tampon with any other block than the one
	/// obtained by get_block_to_feed(). The obtained blocks remains the property
	/// of the fast_tampon object and will be released by it.
	/// Only one block can be obtained at a time, thus get_block_to_feed() and feed()
	/// should be called alternatively. The feeder can call feed_cancel_get_block()
	/// instead of feed() to return the obtained block as if get_block_to_feed() had not been called.
	///
	/// The fetcher interface is almost symmetric, and also has two steps:
	/// - first fetch() a new block of data, read the data from it
	/// - then give back the block to the fast_tampon object calling fetch_recycle().
	/// the Fetcher has not to delete the fetched block nor it must fetch_recycle()
	/// another block than the one that has been fetched.
	/// Only one block can be fetched at a time, thus fetch() and fetch_recycle() should
	/// be called alternatively. The fetcher can call fetch_push_back() instead of fetch_recycle()
	/// to return the fetched block as if fetch() was not called before. The next time, fetch()
	/// will then return the same block that has been fetch_push[ed]_back().
	///
	/// Only on thread can be a feeder, only one (other) thread can be a fetcher.
	///
	/// fast_tampon objects cannot be copied, once created they can only be passed as reference
	/// or using a pointer to them.
	///
	/// \note Class fast_tampon is a template with a single type 'T' as argument. This type is the
	/// base type of the memory block. If you want to exchanges blocks of char between two
	/// threads by use of char * pointers, use tampon<char>


    template <class T> class fast_tampon
    {
    public:
	    /// constructor

	    /// \param[in] max_block is the maximum number of buffers that can be written to without being read
	    /// \param[in] block_size is the maximum size of each buffer
	    /// \note that the object will allocate max_block * block_size * sizeof(T) bytes in consequence
	fast_tampon(unsigned int max_block, unsigned int block_size);

	    // no copy constructor (made private)

	    // no assignment operator (made private)

	    /// the destructor releases all internally allocated blocks even if they have been fetched
	    /// or obtained for feeding.
	~fast_tampon();

	    /// feeder call - step 1

	    /// provides a single block where the caller will be able to write data to in order to be transmitted to the fetcher thread
	    /// \param[out] ptr the address where the caller can write data to
	    /// \param[out] num is the size of the block in number of objects of type T
	    /// \note note that the caller shall never release the address pointed to by ptr
	void get_block_to_feed(T * & ptr, unsigned int & num);

	    /// feeder call - step 2

	    /// Once data has been copied into the block obtained by a call to get_block_to_feed(), use this call to given back this block to the fast_tampon object
	    /// \param[in] ptr address of the block to return to the fast_tampon object
	    /// \param[in] written is the number of element of the block that contain meaninful information, written shall be less than or equal to the argument "num" given by get_block_to_feed().
	void feed(T* ptr, unsigned int written);

	    /// feeder call - step 2 alternative

	    /// put back the block obtained by get_block_to_feed() for example because it was not used.
	    /// This block will be the next one returned by get_block_to_feed
	    /// \param[in] ptr is the address of the block to put back in place for next feed
	void feed_cancel_get_block(T *ptr);

	    /// fetcher call - step 1

	    /// obtain the next block to read
	    /// \param[out] ptr is the address of the data to be read
	    /// \param[out] num is the number of element available for reading
	    /// \note that the caller shall never release the address pointed to by ptr
	void fetch(T* & ptr, unsigned int & num);

	    /// fetcher call - step 2

	    /// Once data has been read, the fetcher must recycle the block into the fast_tampon object
	    /// \param[in] ptr the address of the block to recycle
	void fetch_recycle(T* ptr);

	    /// fetcher call - step 2 alternative

	    /// put back the fetched block if some data remain unfetched for now in this block,
	    /// \param[in] ptr the address of the block to push back into the fast_tampon object
	    /// \param[in] new_num is the new amount of data that is left for reading assuming some data but
	    /// not all could be read from that buffer.
	    /// \note this is the duty of the caller to modify the block for the part of the data
	    /// that has been fetch be suppressed and the unfetched data be moved at the beginning of
	    /// the block. The size is thus modified and provided as new amount of available data in that block
	    /// which will be returned again by the next call to fetch().
	void fetch_push_back(T *ptr, unsigned int new_num);

	    /// to know whether the fast_tampon has objects (readable or skipped)
	bool is_empty() const { return next_feed == next_fetch; };

	    /// to know whether the fast_tampon is *not* empty
	bool is_not_empty() const { return !is_empty(); };

	    /// for feeder to know whether the next call to get_block_to_feed() will be blocking
	bool is_full() const { unsigned int tmp = next_feed; shift_by_one(tmp); return tmp == next_fetch; };

	    /// to know whether the fast_tampon is *not* full
	bool is_not_full() const { return !is_full(); };

	    /// returns the size of the fast_tampon in maximum number of block it can contain
	    ///
	    /// \note this is the max_block argument given at construction time
	unsigned int size() const { return table_size; };

	    /// returns the allocation size of each block
	    ///
	    /// \note this is the block_size argument given at construction time
	unsigned int block_size() const { return alloc_size; };

	    /// reset the object fields and mutex as if the object was just created
	void reset();

    private:
            /// copy constructor is disabled and generates an exception if called
        fast_tampon(const fast_tampon & ref) { throw THREADAR_BUG; };

            /// assignment operator is disabled and generates an exception if called
        const fast_tampon & operator = (const fast_tampon & ref) { throw THREADAR_BUG; };

        struct atom
        {
            T* mem;
            unsigned int data_size;

            atom() { mem = nullptr; data_size = 0; };
        };

        mutex modif;              //< protect the access to table
        atom *table;              //< datastructure holding data in transit between two threads
        unsigned int table_size;  //< size of table, i.e. number of struct atom it holds
        unsigned int alloc_size;  //< size of allocated memory for each atom in table
        unsigned int next_feed;   //< index in table of the next atom to use for feeding table
        unsigned int next_fetch;  //< index in table of the next atom to use for fetch table
        bool fetch_outside;       //< if set to true, table's index pointed to by next_fetch is used by the fetcher
        bool feed_outside;        //< if set to true, table's index pointed to by next_feed is used by the feeder
        condition waiting_feeder; //< feeder thread may be stuck waiting on that semaphore if table is full
        condition waiting_fetcher;//< fetcher thread may be stuck waiting on that semaphore if table is empty
        bool feeder_go_lock;      //< true to inform fetcher than feeder is about to or has already acquire lock on waiting_feeder
        bool fetcher_go_lock;     //< true to inform feeder than fetcher is about to or has already acquire lock on waiting_fetcher

            /// cyclicly shift an index (next_feed or next_fetch) by one position
        void shift_by_one(unsigned int & x) const;

    };

    template <class T> fast_tampon<T>::fast_tampon(unsigned int max_block, unsigned int block_size)
    {
	table_size = max_block;
	table = new atom[table_size];
	if(table == nullptr)
	    throw exception_memory();
	try
	{
	    alloc_size = block_size;
	    try
	    {
		for(unsigned int i = 0 ; i < table_size ; ++i)
		{
		    table[i].mem = new T[alloc_size];
		    if(table[i].mem == nullptr)
			throw exception_memory();
		    table[i].data_size = 0;
		}
		reset();
	    }
	    catch(...)
	    {
		for(unsigned int i = 0; i < table_size ; ++i)
		{
		    if(table[i].mem != nullptr)
			delete [] table[i].mem;
		}

		throw;
	    }
	}
	catch(...)
	{
	    if(table != nullptr)
		delete [] table;
	    throw;
	}
    }


    template <class T> fast_tampon<T>::~fast_tampon()
    {
	if(table != nullptr)
	{
	    for(unsigned int i = 0 ; i < table_size ; ++i)
	    {
		if(table[i].mem != nullptr)
		    delete [] table[i].mem;
	    }
	    delete [] table;
	}
    }

    template <class T> void fast_tampon<T>::get_block_to_feed(T * & ptr, unsigned int & num)
    {
        if(feed_outside)
            throw exception_range("feed already out!");

        if(is_full()) // if not full it will not become full, no need to enter a critical section
        {
            waiting_feeder.lock();  // --- critical section START
            if(is_full()) // still full thus we must wait
            {
                feeder_go_lock = true; // inform fetcher that we will suspend in waiting_feeder
                waiting_feeder.wait(); // this will suspend the thread when calling unlock()
            }
            waiting_feeder.unlock(); // --- critical section END
        }

        if(is_full())
            throw THREADAR_BUG; // still full!?!

        feed_outside = true;
        ptr = table[next_feed].mem;
        num = alloc_size;
    }

    template <class T> void fast_tampon<T>::feed(T *ptr, unsigned int num)
    {
        if(!feed_outside)
            throw exception_range("fetch not outside!");
        feed_outside = false;

        if(ptr != table[next_feed].mem)
            throw exception_range("returned ptr is not the one given earlier for feeding");
        table[next_feed].data_size = num;

        modif.lock();   // --- critical section START
        shift_by_one(next_feed);
        modif.unlock(); // --- critical section END

        if(fetcher_go_lock)
        {
            waiting_fetcher.lock();
            fetcher_go_lock = false;
            waiting_fetcher.signal();
            waiting_fetcher.unlock();
        }
    }

    template <class T> void fast_tampon<T>::feed_cancel_get_block(T *ptr)
    {
	if(!feed_outside)
	    throw exception_range("feed not outside!");
	feed_outside = false;
	if(ptr != table[next_feed].mem)
	    throw exception_range("returned ptr is not the one given earlier for feeding");
    }

    template <class T> void fast_tampon<T>::fetch(T* & ptr, unsigned int & num)
    {
        if(fetch_outside)
            throw exception_range("already fetched block outside");

        if(is_empty()) // if not empty it will not become empty, no need to enter critical section
        {
            waiting_fetcher.lock();   // --- critical section START
            if(is_empty())
            {
                fetcher_go_lock = true;    // to inform feeder that we will suspend on waiting_fetcher
                waiting_fetcher.wait(); // this will suspend the thread when calling unlock()
            }
            waiting_fetcher.unlock(); // --- critical section END
        }

        if(is_empty())
            throw THREADAR_BUG; // still empty!?!

        fetch_outside = true;
        ptr = table[next_fetch].mem;
        num = table[next_fetch].data_size;
    }

    template <class T> void fast_tampon<T>::fetch_recycle(T* ptr)
    {
        if(!fetch_outside)
            throw exception_range("no block outside for fetching");
        fetch_outside = false;
        if(ptr != table[next_fetch].mem)
            throw exception_range("returned ptr is no the one given earlier for fetching");

        modif.lock();   // --- critical section START
        shift_by_one(next_fetch);
        modif.unlock(); // --- critical section END

        if(feeder_go_lock)
        {
            waiting_feeder.lock();
            feeder_go_lock = false;
            waiting_feeder.signal();
            waiting_feeder.unlock();
        }
    }

    template <class T> void fast_tampon<T>::fetch_push_back(T* ptr, unsigned int new_num)
    {
	if(!fetch_outside)
	    throw exception_range("no block outside for fetching");
	fetch_outside = false;

	if(ptr != table[next_fetch].mem)
	    throw exception_range("returned ptr is not the one given earlier for fetching");
	table[next_fetch].data_size = new_num;
    }


    template <class T> void fast_tampon<T>::reset()
    {
        next_feed = 0;
        next_fetch = 0;
        fetch_outside = false;
        feed_outside = false;
        feeder_go_lock = false;
        fetcher_go_lock = false;
        (void)waiting_feeder.try_lock();
        waiting_feeder.signal();
        waiting_feeder.unlock();
        (void)waiting_fetcher.try_lock();
        waiting_fetcher.signal();
        waiting_fetcher.unlock();
    }

    template <class T> void fast_tampon<T>::shift_by_one(unsigned int & x) const
    {
	++x;
	if(x >= table_size)
	    x = 0;
    }

    	/// \example ../doc/examples/fast_tampon_example.cpp
	/// this is an example of use of class libthreadar::fast_tampon and
	/// libthreadar::exception_base and derivated classes

} // end of namespace

#endif


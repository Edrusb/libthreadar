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

#ifndef LIBTHREADAR_TAMPON_H
#define LIBTHREADAR_TAMPON_H

    /// \file tampon.hpp
    /// \brief defines the tampon class that provides asynchronous pipe-like (unidirectional) communication between two threads

#include "config.h"

    // C system headers
extern "C"
{

}
    // C++ standard headers


    // libthreadar headers
#include "mutex.hpp"
#include "exceptions.hpp"

namespace libthreadar
{

	///  DEPRECATED see fast_tampon instead!

	/// Class tampon provides asynchronous communication between two threads
	///
	/// A first thread is defined as a *feeder* and feeds the tampon object with data
	/// that the other thread known as the *fetcher* will read at a later time.
	/// If the tampon is empty the fetcher thread is suspended, if it is full
	/// the feeder thread is suspended. Feeding an empty
	/// tampon automatically awakes the fetcher thread if it was suspended for reading,
	/// reading a full tampon automatically awakes a feeder that was suspended
	/// for writing.
	///
	/// The feeder has to proceed in two steps:
	/// - first call get_block_to_feed() and write data to the obtained block
	/// - once the data has been written to the block, it must call feed() with that block.
	/// The feeder must not feed() the tampon with any other block than the one
	/// obtained by get_block_to_feed(). The obtained blocks remains the property
	/// of the tampon object and will be released by it.
	/// Only one block can be obtained at a time, thus get_block_to_feed() and feed()
	/// should be called alternatively. The feeder can call feed_cancel_get_block()
	/// instead of feed() to return the obtained block as if get_block_to_feed() had not been called.
	///
	/// The fetcher interface is almost symmetric, and also has two steps:
	/// - first fetch() a new block of data, read the data from it
	/// - then give back the block to the tampon object calling fetch_recycle().
	/// the Fetcher has not to delete the fetched block nor it must fetch_recycle()
	/// another block than the one that has been fetched.
	/// Only one block can be fetched at a time, thus fetch() and fetch_recycle() should
	/// be called alternatively. The fetcher can call fetch_push_back() instead of fetch_recycle()
	/// to return the fetched block as if fetch() was not called before. The next time, fetch()
	/// will then return the same block that has been fetch_push[ed]_back().
	///
	/// Only on thread can be a feeder, only one (other) thread can be a fetcher.
	///
	/// tampon objects cannot be copied, once created they can only be passed as reference
	/// or using a pointer to them.
	///
	/// \note Class tampon is a template with a single type 'T' as argument. This type is the
	/// base type of the memory block. If you want to exchanges blocks of char between two
	/// threads by use of char * pointers, use tampon<char>
	///
	/// The *fetcher* has the possiblity to read data after the next block to be fetched:
	/// - same as before, the fetcher has first to fetch() a block
	/// - then calling fetch_push_back_and_skip() the block is put back into the pipe but the next call
	/// to fetch() will provide the block after this one if there is one available. If no other block
	/// is available the calling thread will be suspended up to the feeder provides a new block of data. The block
	/// that has been put back this way to the tampon object is preserved.
	/// - once the next block is obtained by fetch() it can be remove from the pipe calling fetch_recycle(),
	/// put back in place and available for next fetch() calling fetch_push_back() or put back and having
	/// the cursor skipped one step further calling fetch_push_back_and_skip().
	///
	/// In that situation the block(s) at the head of the pipe (which is/are the one(s) for which fetch_push_back_and_skip()
	/// has been used) is/are still there but not readable. The fetcher has to call fetch_skip_back() to return
	/// the cursor at the beginning of the pipe in order to have access to them and possibly remove them from the pipe.
    template <class T> class tampon
    {
    public:
	    /// constructor

	    /// \param[in] max_block is the maximum number of buffers that can be written to without being read
	    /// \param[in] block_size is the maximum size of each buffer
	    /// \note that the object will allocate max_block * block_size * sizeof(T) bytes in consequence
	tampon(unsigned int max_block, unsigned int block_size);

	    /// no copy constructor
	tampon(const tampon & ref) = delete;

	    /// no move constructor
	tampon(tampon && ref) noexcept = delete;

	    /// no assignment operator
	tampon & operator = (const tampon & ref) = delete;

	    /// no move operator
	tampon & operator = (tampon && ref) noexcept = delete;

	    /// the destructor releases all internally allocated blocks even if they have been fetched
	    /// or obtained for feeding.
	~tampon();

	    /// feeder call - step 1

	    /// provides a single block where the caller will be able to write data to in order to be transmitted to the fetcher thread
	    /// \param[out] ptr the address where the caller can write data to
	    /// \param[out] num is the size of the block in number of objects of type T
	    /// \note note that the caller shall never release the address pointed to by ptr
	void get_block_to_feed(T * & ptr, unsigned int & num);

	    /// feeder call - step 2

	    /// Once data has been copied into the block obtained by a call to get_block_to_feed(), use this call to given back this block to the tampon object
	    /// \param[in] ptr address of the block to return to the tampon object
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

	    /// Once data has been read, the fetcher must recycle the block into the tampon object
	    /// \param[in] ptr the address of the block to recycle
	void fetch_recycle(T* ptr);

	    /// fetcher call - step 2 alternative

	    /// put back the fetched block if some data remain unfetched for now in this block,
	    /// \param[in] ptr the address of the block to push back into the tampon object
	    /// \param[in] new_num is the new amount of data that is left for reading assuming some data but
	    /// not all could be read from that buffer.
	    /// \note this is the duty of the caller to modify the block for the part of the data
	    /// that has been fetch be suppressed and the unfetched data be moved at the beginning of
	    /// the block. The size is thus modified and provided as new amount of available data in that block
	    /// which will be returned again by the next call to fetch().
	void fetch_push_back(T *ptr, unsigned int new_num);

	    /// put back the fetched block and skip to next block for the next fetch()

	    /// \param[in] ptr address of the fetched block to push back
	    /// \param[in] new_num possibily modified size of the fetched block to push back
	void fetch_push_back_and_skip(T *ptr, unsigned int new_num);

	    /// reactivate all skipped blocks, next fetch() will be the oldest available block
	void fetch_skip_back();

	    /// to known whether next fetch will be blocking (not skipped blocks)
	bool has_readable_block_next() const;

	    /// to know whether the tampon has objects (readable or skipped)
	bool is_empty() const;

	    /// to know whether the tampon is *not* empty
	bool is_not_empty() const { return !is_empty(); };

	    /// for feeder to know whether the next call to get_block_to_feed() will be blocking
	bool is_full() const { return full; }; // no need to acquire mutex "modif"

	    /// to know whether the tampon is *not* full
	bool is_not_full() const { return !is_full(); };

	    /// returns true if only one slot is available before filling the tampon
	bool is_quiet_full() const { unsigned int tmp = next_feed; shift_by_one(tmp); return tmp == fetch_head; };

	    /// returns the size of the tampon in maximum number of block it can contain

	    /// \note this is the max_block argument given at construction time
	unsigned int size() const { return table_size; };

	    /// returns the allocation size of each block

	    /// \note this is the block_size argument given at construction time
	unsigned int block_size() const { return alloc_size; };

	    /// returns the current number of blocks currently used in the tampon (fed but not fetched)
	unsigned int load() const { return fetch_head <= next_feed ? next_feed - fetch_head : table_size - (fetch_head - next_feed); };

	    /// reset the object fields and mutex as if the object was just created
	void reset();

    private:

	struct atom
	{
	    T* mem;
	    unsigned int data_size;

	    atom() { mem = nullptr; data_size = 0; };
	};

	mutex modif;              //< to make critical section when non atomic action requires a status has not changed between a test and following action
	atom *table;              //< datastructure holding data in transit between two threads
	unsigned int table_size;  //< size of table, i.e. number of struct atom it holds
	unsigned int alloc_size;  //< size of allocated memory for each atom in table
	unsigned int next_feed;   //< index in table of the next atom to use for feeding table
	unsigned int next_fetch;  //< index in table of the next atom to use for fetch table
	unsigned int fetch_head;  //< the oldest object to be fetched
	bool fetch_outside;       //< if set to true, table's index pointed to by next_fetch is used by the fetcher
	bool feed_outside;        //< if set to true, table's index pointed to by next_feed is used by the feeder
	mutex waiting_feeder;     //< feeder thread may be stuck waiting on that semaphore if table is full
	mutex waiting_fetcher;    //< fetcher thread may be stuck waiting on that semaphore if table is empty
	bool full;                //< set when tampon is full
	bool feeder_go_lock;      //< true to inform fetcher than feeder is about to or has already acquire lock on waiting_feeder
	bool feeder_lock_track;   //< only used by feeder to lock on waiting_feeder once outside of critical section
	bool fetcher_go_lock;     //< true to inform feeder than fetcher is about to or has already acquire lock on waiting_fetcher
	bool fetcher_lock_track;  //< only used by fetcher to lock on waiting_fetcher once outside of critical section

	bool is_empty_no_lock() const { return next_feed == fetch_head && !full; };

	    /// for fetcher to know whether the next fetch will be blocking
	bool has_readable_block_next_no_lock() const { return next_feed != next_fetch || full; }

	    /// cyclicly shift an index (next_feed or next_fetch) by one position
	void shift_by_one(unsigned int & x) const;

	    /// cyclicly shift an index by one in the other direction (backbward) than shift_by_one() does
	void shift_back_by_one(unsigned int & x) const;

	    /// shift table data by one in the given range [begin, end) and return the new index for "end" (first item out of range)
	    ///
	    /// \param[in] begin is the index of the first slot that will be copied to the previous slot (cyclicly)
	    /// \param[in] end is the first slot that will *not* be copied to the previous slot (cyclicly)
	void shift_by_one_data_in_range(unsigned int begin, unsigned int end);

    };

    template <class T> tampon<T>::tampon(unsigned int max_block, unsigned int block_size)
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


    template <class T> tampon<T>::~tampon()
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

    template <class T> void tampon<T>::get_block_to_feed(T * & ptr, unsigned int & num)
    {
	if(feed_outside)
	    throw exception_range("feed already out!");

	modif.lock();  	// --- critical section START
	if(is_full())
	{
	    feeder_go_lock = true;   // inform fetcher that we will suspend in waiting_feeder
	    feeder_lock_track = true;// to suspend on waiting_feeder once we will be out of the critical section
	}
	modif.unlock(); // --- critical section END

	if(feeder_lock_track)
	{
	    feeder_lock_track = false;
	    waiting_feeder.lock(); // cannot lock inside a critical section ...
	}

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
	feed_outside = false;

	if(ptr != table[next_feed].mem)
	    throw exception_range("returned ptr is not the one given earlier for feeding");
	table[next_feed].data_size = num;

	modif.lock();   // --- critical section START
	shift_by_one(next_feed);
	if(next_feed == fetch_head)
	    full = true;
	if(fetcher_go_lock)
	{
	    fetcher_go_lock = false;
	    waiting_fetcher.unlock();
	}
	modif.unlock(); // --- critical section END
    }

    template <class T> void tampon<T>::feed_cancel_get_block(T *ptr)
    {
	if(!feed_outside)
	    throw exception_range("feed not outside!");
	feed_outside = false;
	if(ptr != table[next_feed].mem)
	    throw exception_range("returned ptr is not the one given earlier for feeding");
    }

    template <class T> void tampon<T>::fetch(T* & ptr, unsigned int & num)
    {
	if(fetch_outside)
	    throw exception_range("already fetched block outside");

	modif.lock();   // --- critical section START
	if(!has_readable_block_next_no_lock())
	{
	    fetcher_go_lock = true;    // to inform feeder that we will suspend on waiting_fetcher
	    fetcher_lock_track = true; // to suspend on waiting_fetcher once we will be out of the critical section
	}
	modif.unlock(); // --- critical section END

	if(fetcher_lock_track)
	{
	    fetcher_lock_track = false;
	    waiting_fetcher.lock();   // cannot lock inside a critical section ...
	}

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
	fetch_outside = false;
	if(ptr != table[next_fetch].mem)
	    throw exception_range("returned ptr is no the one given earlier for fetching");

	modif.lock();   // --- critical section START
	if(next_fetch == fetch_head)
	{

		// no block were skipped

	    shift_by_one(fetch_head);
	    next_fetch = fetch_head;
	    full = false;
	}
	else
	{
	    unsigned int tmp = next_fetch;
	    atom tmp_tom;

	    shift_by_one(tmp);
	    shift_by_one_data_in_range(tmp, next_feed);

		// we also take into account the situation
		// where a block has been given for feeding
		// so the next call to feed() will match the
		// expected address of the returned block
	    tmp = next_feed; // recording old position of next_feed
	    shift_back_by_one(next_feed);
		// swapping contents between old next_feed position
		// and new one:
	    tmp_tom = table[next_feed];
	    table[next_feed] = table[tmp];
	    table[tmp] = tmp_tom;
		// done!

	    full = false;
	}

	if(feeder_go_lock)
	{
	    feeder_go_lock = false;
	    waiting_feeder.unlock();
	}
	modif.unlock(); // --- critical section END
    }

    template <class T> void tampon<T>::fetch_push_back(T* ptr, unsigned int new_num)
    {
	if(!fetch_outside)
	    throw exception_range("no block outside for fetching");
	fetch_outside = false;

	if(ptr != table[next_fetch].mem)
	    throw exception_range("returned ptr is not the one given earlier for fetching");
	table[next_fetch].data_size = new_num;
    }

    template <class T> void tampon<T>::fetch_push_back_and_skip(T *ptr,
								unsigned int new_num)
    {
	fetch_push_back(ptr, new_num);
      	modif.lock();   // --- critical section START
	if(full && next_fetch == next_feed) // reach last block feed, cannot skip it
	    throw exception_range("cannot skip the last fed block when the tampon is full");
	shift_by_one(next_fetch);
	modif.unlock(); // --- critical section END
    }

    template <class T> void tampon<T>::fetch_skip_back()
    {
	if(fetch_outside)
	    throw exception_range("cannot skip back fetching while a block is being fetched");
	next_fetch = fetch_head;
    }


    template <class T> bool tampon<T>::has_readable_block_next() const
    {
	bool ret;

	tampon<T> *me = const_cast<tampon<T> *>(this);
	if(me == nullptr)
	    throw THREADAR_BUG;
	me->modif.lock();
	ret = has_readable_block_next_no_lock();
	me->modif.unlock();

	return ret;
    }


    template <class T> bool tampon<T>::is_empty() const
    {
	bool ret;

	tampon<T> * me = const_cast<tampon<T> *>(this);
	if(me == nullptr)
	    throw THREADAR_BUG;
	me->modif.lock();
	ret = is_empty_no_lock();
	me->modif.unlock();

	return ret;
    }

    template <class T> void tampon<T>::reset()
    {
	next_feed = 0;
	next_fetch = 0;
	fetch_head = 0;
	fetch_outside = false;
	feed_outside = false;
	full = false;
	feeder_go_lock = false;
	feeder_lock_track = false;
	fetcher_go_lock = false;
	fetcher_lock_track = false;
	(void)waiting_feeder.try_lock();
	(void)waiting_fetcher.try_lock();
    }

    template <class T> void tampon<T>::shift_by_one(unsigned int & x) const
    {
	++x;
	if(x >= table_size)
	    x = 0;
    }

    template <class T> void tampon<T>::shift_back_by_one(unsigned int & x) const
    {
	if(x == 0)
	    x = table_size - 1;
	else
	    --x;
    }

    template <class T> void tampon<T>::shift_by_one_data_in_range(unsigned int begin, unsigned int end)
    {

	if(begin != end)
	{
	    unsigned int prev = begin;
	    shift_back_by_one(prev);
	    T* not_squeezed = table[prev].mem; // we will erase the address pointed to by mem so we keep it in memory here

	    while(begin != end)
	    {
		table[prev] = table[begin]; // this copies both mem (the value of the pointer, not the pointed to) and data_size,
		prev = begin;
		shift_by_one(begin);
	    }

	    table[prev].mem = not_squeezed;
	    table[prev].data_size = 0; // by precaution
	}
    }


} // end of namespace

#endif


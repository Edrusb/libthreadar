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

#ifndef LIBTHREADAR_RATELIER_HPP
#define LIBTHREADAR_RATELIER_HPP

    /// \file ratelier.hpp
    /// \brief defines structure that is suitable to dispatch or gather data between many workers
    ///
    /// many worker can get each an oject in a slot while a main thread feeds the objects in the ratelier
    /// or many workers can push objects in the ratelier while a main thread gather/removes them in order
    /// a set of worker can also use two rateliers, one they fetch object from with an index order and
    /// an second they push their work into with that same index order. The main thread push objects of
    /// chunk of data in order into the first ratelier and gather the result respecting this same order
    /// from the second ratelier

#include "config.h"

    // C system headers
extern "C"
{
}
    // C++ standard headers
#include <vector>
#include <map>
#include <deque>
#include <memory>

    // libthreadar headers
#include "mutex.hpp"


namespace libthreadar
{
        /// the class ratelier is a fixed length range of slots of arbitrary defined object type

	/// the number of slot should be greater than the number of worker that
	/// fetch data from it or put data to it (two different modes of operation)
	/// - in the first mode, worker fetch an object use it and release it which
	///   empties the slot. While a non-worker thread feed the ratelier with new
	///   objects. Each object taken from the ratelier by a worker is given an index
	///   number for the worker can put this object in a given slot of another ratelier
	/// - in the second mode, worker put objects at a given slot and a non-worker
	///   thread get the objects in order until a slot is empty, this releases the
	///   those objects from the ratelier
	/// .
	/// the first slot starts with index zero. Upon operation the ratelier is filled with
	/// and is voided from objects at different slots. Never a slot is used twice, the
	/// index of the first slot always increases in order to hold a window on given size
	/// object slot on an virtually infinint sequence of slots.
	/// As the ratelier can hold a maximum of object, the non-worker thread may be suspending
	/// in the first mode until a worker releases an slot, and a worker thread may be
	/// suspended until the non-worker thread reads the ratelier.

    template <class T> class ratelier
    {
	ratelier(unsigned int size, T objtype);
	ratelier(const ratelier & ref) = delete;
	ratelier(ratelier && ref) = delete;
	ratelier & operator = (const ratelier & ref) = delete;
	ratelier & operator = (ratelier && ref) = delete;
	virtual ~ratelier() = default;

	    /// mean for the non-worker thread to provide data to the ratelier (mode 1)
	    ///
	    /// \the data is added to new higher indexs of the virtualluy infininte
	    /// list of object. However the caller may be suspended if the ratelier
	    /// is full
	void feed(std::unique_ptr<T> & one);

	    /// mean for a worker thread to obtain a object in the lowest slot available (mode 1)
	    ///
	    /// \return the slot number that will be needed to release the slot once work is completed
	    /// \note this call may suspended the caller until the non-worker thread feeds
	    /// the ratelier with new data
	std::unique_ptr<T> worker_get_one(unsigned int & slot);

	    /// release the slot previously obtained by get_one() (mode 1)
	    ///
	    /// \note there is no control on the correctness of the slot asked for release
	    /// in regard of the calling thread, however if the slot is already empty an
	    /// exception is thrown
	void worker_release(unsigned int slot);

	    /// provides a worker thread a mean to given slot to the ratelier (mode 2)
	    ///
	    /// \note if the slot is already full an exception is thrown
	    /// \note if the ratelier is full the caller will be suspended until the
	    /// non-worker thread calls get() to make some room
	void worker_fill(unsigned int slot, std::unique_ptr<T> & one);

	    /// obtain the lowest continuous filled slots of the ratelier and free them (mode 2)
	void get(std::vector<std::unique_ptr<T> > & ones);

    private:
	struct slot
	{
	    std::unique_ptr<T> obj;
	    bool empty;
	    bool locked;
	    unsigned int index;

	    slot() { empty = true; locked = false; };
	};

	unsigned int first_index; ///< index of the first slot (lowest index) used
	unsigned int next_index; ///< index of the next slot to use (always increases but may overflood)
	std::vector<slot> table; ///< table of slots to store data
	std::map<unsigned int, unsigned int> corres; ///< associate infinite range index to index in table
	std::deque<unsigned int> empty_slot; ///< empty slot of table
	libthreadar::condition verrou;  ///< lock to manipulate private data

	void updating_first_index(); ///< scan the table for the smallest index and updates first_index field
    };

    template <class T> ratelier<T>::ratelier(unsigned int size, T objtype): table(size), corres(size)
    {
	next_index = 0;
	first_index = 0;

	for(unsigned int i = 0; i < size; ++i)
	    empty_slot.push_back(i);
    }

    template <class T> void ratelier<T>::feed(std::unique_ptr<T> & one)
    {
	unsigned int tableindex;

	verrou.lock();
	try
	{
	    if(empty_slot.empty())
		verrou.wait();

	    if(empty_slot.empty())
		throw THREADAR_BUG;
	    tableindex = empty_slot.back();

		// sanity checks

	    if(tableindex > table.size())
		throw THREADAR_BUG;
	    if(!table[tableindex].empty)
		throw THREADAR_BUG;
	    if(table[tableindex].locked)
		throw THREADAR_BUG;

		// recording the change

	    table[tableindex].empty = false;
	    table[tableindex].obj = one;
	    table[tableindex].index = next_index;

	    corres[next_index] = tableindex;
	    ++next_index;

	    empty_slot.pop_back();
	    verrou.signal(); // awake one worker thread possibily suspended if ratelier was empty
	}
	catch(...)
	{
	    verrou.unlock();
	    throw;
	}
	verrou.unlock();
    }

    template <class T> std::unique_ptr<T> ratelier<T>::worker_get_one(unsigned int & slot)
    {
	std::unique_ptr<T> ret;

	verrou.lock();
	try
	{
	    while(first_index >= next_index) // no data available
		verrou.wait(); // current thread may be awaken by another worker while releasing a slot

	    if(first_index >= next_index)
		throw THREADAR_BUG;
	    else
	    {
		std::map<unsigned int, unsigned int>::iterator it = corres.find(first_index);

		if(it == corres.end())
		    throw THREADAR_BUG;

		    // sanity checks

		if(it->second > table.size())
		    throw THREADAR_BUG;
		if(table[it->second].locked)
		    throw THREADAR_BUG;
		if(table[it->second].empty)
		    throw THREADAR_BUG;
		if(!table[it->second].obj)
		    throw THREADAR_BUG;

		    // recording the change

		table[it->second].locked = true;
		table[it->second].index = first_index;
		ret = table[it->second].obj;
		slot = first_index;
	    }
	}
	catch(...)
	{
	    verrou.unlock();
	    throw;
	}
	verrou.unlock();

	return ret;
    }

    template <class T> void ratelier<T>::worker_release(unsigned int slot)
    {
	verrou.lock();

	try
	{
	    std::map<unsigned int, unsigned int>::iterator it = corres.find(slot);

	    if(it == corres.end())
		throw exception_range("the ratelier index to release is unknown");

		// sanity checks

	    if(it->second > table.size())
		throw THREADAR_BUG;
	    if(table[it->second].empty)
		throw THREADAR_BUG;
	    if(!table[it->second].locked)
		throw THREADAR_BUG;
	    if(table[it->second].obj)
		throw THREADAR_BUG;
	    if(table[it->second].index != slot)
		throw THREADAR_BUG;

		// recording the change

	    table[it->second].empty = true;
	    table[it->second].locked = false;

		// awaking non-worker thread eventually pending for a free slot
		// this will be done at verrou.unlock() time
	    if(empty_slot.empty())
		verrou.broadcast();

		// reusing quicker the last block used
		// as the back() be used first
	    empty_slot.push_back(it->second);
	    corres.erase(it); // removing the correspondance

	    updating_first_index();
	}
	catch(...)
	{
	    verrou.unlock();
	    throw;
	}
	verrou.unlock();
    }

    template <class T> void ratelier<T>::worker_fill(unsigned int slot, std::unique_ptr<T> & one)
    {
	verrou.lock();

	try
	{
	    std::map<unsigned int, unsigned int>::iterator it = corres.find(slot);
	    unsigned int index;

	    if(it != corres.end())
		throw exception_range("the ratelier index to fill is already used");

	    while(empty_slot.empty()) // no free slot available
		verrou.wait(); // can be awake by another worker thread

	    index = empty_slot.back();

		// sanity checks

	    if(index > table.size())
		throw THREADAR_BUG;
	    if(!table[index].empty)
		throw THREADAR_BUG;
	    if(table[index].locked)
		throw THREADAR_BUG;

		// recording the change

	    corres[slot] = index;
	    table[index].obj = one;
	    table[index].empty = false;
	    table[index].index = slot;

	    if(next_index <= slot)
		next_index = slot + 1;

	    if(empty_slot.size() == table.size()) // all slots were empty
		verrou.broadcast(); // awaking non-worker thread possibly waiting for data

	    empty_slot.pop_back();
	}
	catch(...)
	{
	    verrou.unlock();
	    throw;
	}
	verrou.unlock();
    }

    template <class T> void ratelier<T>::get(std::vector<std::unique_ptr<T> > & ones)
    {
	unsigned int index = first_index;

	ones.clear();

	verrou.lock();
	try
	{
	    while(index < next_index)
	    {
		std::map<unsigned int, unsigned int>::iterator it = corres.find(index);

		if(it == corres.end())
		    index = next_index; // ending the process
		else
		{
			// sanity checks

		    if(it->second > table.size())
			throw THREADAR_BUG;
		    if(table[it->second].index != index)
			throw THREADAR_BUG;
		    if(table[it->second].locked)
			throw THREADAR_BUG;
		    if(table[it->second].empty)
			throw THREADAR_BUG;
		    if(!table[it->second].obj)
			throw THREADAR_BUG;

			// recording the change

		    ones.push_back(table[it->second].obj);
		    table[it->second].empty = true;
		    empty_slot.push_back(it->second);
		    corres.erase(it);
		}
		++index;
	    }
	    updating_first_index();
	}
	catch(...)
	{
	    verrou.unlock();
	    throw;
	}
	verrou.unlock();
    }

} // end of namespace

#endif

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

#ifndef LIBTHREADAR_RATELIER_GATHER_HPP
#define LIBTHREADAR_RATELIER_GATHER_HPP

    /// \file ratelier_gather.hpp
    /// \brief defines structure that is suitable to gather data from many workers
    ///
    /// each worker can fill its job result to the ratelier_gather with an index number.
    /// This index is used for to provide the jobs in sequence to the gathering thread

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
        /// the class ratelier_gather has a fixed length range of slots of arbitrary defined object type

	/// the number of slot should be greater than the number of workers that
	/// fill the ratelier with data
	/// workers put objects at a given slot and a non-worker thread (or gathering thread)
	/// get those objects in order, this releases the corresponding slots of the ratelier_gather

    template <class T> class ratelier_gather
    {
    public:
	ratelier_gather(unsigned int size);
	ratelier_gather(const ratelier_gather & ref) = delete;
	ratelier_gather(ratelier_gather && ref) noexcept = default;
	ratelier_gather & operator = (const ratelier_gather & ref) = delete;
	ratelier_gather & operator = (ratelier_gather && ref) noexcept = default;
	virtual ~ratelier_gather() = default;

	    /// provides a worker thread a mean to given slot to the ratelier_gather
	    ///
	    /// \note if the slot is already full an exception is thrown
	    /// \note if the ratelier_gather is full the caller will be suspended until the
	    /// non-worker thread calls get() to make some room
	void worker_push_one(unsigned int slot, std::unique_ptr<T> one);

	    /// obtain the lowest continuous filled slots of the ratelier_gather and free them
	void gather(std::vector<std::unique_ptr<T> > & ones);

    private:
	struct slot
	{
	    std::unique_ptr<T> obj;
	    bool empty;
	    unsigned int index;

	    slot() { empty = true; };
	};

	unsigned int next_index; ///< next index to start the next gather() with
	std::vector<slot> table; ///< table of slots to store data
	std::map<unsigned int, unsigned int> corres; ///< associate infinite range index to index in table
	std::deque<unsigned int> empty_slot; ///< empty slot of table
	libthreadar::condition verrou;  ///< lock to manipulate private data
    };

    template <class T> ratelier_gather<T>::ratelier_gather(unsigned int size): table(size)
    {
	next_index = 0;

	for(unsigned int i = 0; i < size; ++i)
	    empty_slot.push_back(i);
    }


    template <class T> void ratelier_gather<T>::worker_push_one(unsigned int slot, std::unique_ptr<T> one)
    {
	verrou.lock();

	try
	{
	    while(empty_slot.empty()) // no free slot available
	    {
		std::map<unsigned int, unsigned int>::iterator it = corres.begin();
		if(it == corres.end())
		    throw THREADAR_BUG; /// no empty slot but not correspondance with any used slot!
		else
		{
		    if(it->first > next_index)
			throw exception_range("ratelier_gather is full but has not the first expected slot needed to extract some data form it, dead-lock situation would arise, aborting");
		}
		verrou.wait(); // we can be awaken by another worker thread, so we loop to check the condition has changed
	    }

	    std::map<unsigned int, unsigned int>::iterator it = corres.find(slot);
	    unsigned int index;

	    if(it != corres.end())
		throw exception_range("the ratelier_gather index to fill is already used");

	    index = empty_slot.back();

		// sanity checks

	    if(index >= table.size())
		throw THREADAR_BUG;
	    if( ! table[index].empty)
		throw THREADAR_BUG;

		// recording the change

	    corres[slot] = index;
	    table[index].obj = std::move(one);
	    table[index].empty = false;
	    table[index].index = slot;

	    if(empty_slot.size() == table.size()) // all slots were empty
		verrou.broadcast(); // awaking non-worker thread possibly waiting for data

	    empty_slot.pop_back();
	}
	catch(...)
	{
	    verrou.broadcast();
	    verrou.unlock();
	    throw;
	}
	verrou.unlock();
    }

    template <class T> void ratelier_gather<T>::gather(std::vector<std::unique_ptr<T> > & ones)
    {
	ones.clear();

	verrou.lock();
	try
	{
	    std::map<unsigned int, unsigned int>::iterator it;
	    std::map<unsigned int, unsigned int>::iterator tmp;
	    bool was_full = empty_slot.empty();

	    do
	    {
		it = corres.begin();

		while(it != corres.end())
		{
		    if(it->first > next_index) // not continuous sequence
		    {
			if(ones.empty())
			    verrou.wait(); // waiting for a worker to add the expected slot
			break; // exiting the inner while loop
		    }

		    if(it->first == next_index)
		    {

			    // sanity checks

			if(it->second >= table.size())
			    throw THREADAR_BUG;
			if(table[it->second].index != next_index)
			    throw THREADAR_BUG;
			if(table[it->second].empty)
			    throw THREADAR_BUG;
			if( ! table[it->second].obj)
			    throw THREADAR_BUG;

			    // recording the change

			ones.push_back(std::move(table[it->second].obj));
			table[it->second].empty = true;
			empty_slot.push_back(it->second);
			tmp = it;
			++it;
			corres.erase(tmp);
			++next_index;
		    }
		    else // integer overload occured for the index
		    {
			++it; // skipping this entry
			if(it == corres.end())
			    verrou.wait();
			    // avoiding endless loop but rather
			    // waiting for a feeder to add some data
		    }
		}
	    }
	    while(ones.empty());

	    if(was_full)
		verrou.broadcast(); // awake all workers pending for some room
	}
	catch(...)
	{
	    verrou.broadcast();
	    verrou.unlock();
	    throw;
	}
	verrou.unlock();
    }

} // end of namespace

#endif

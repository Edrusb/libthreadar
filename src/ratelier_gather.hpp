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
    /// and starts to zero. If a index is missing no data is delivered to the gathering
    /// thread until a worker thread provide it. The gathering thread is thus garantied
    /// that whatever the execution order of workers their resulting job is provided
    /// in sequence to the gathering thread.
    ///
    /// the original design for this class is to work in conjunction
    /// with a ratelier_scatter when a sequences data has to be processed
    /// by many workers but the resulting of the process should also be
    /// ordered following the original data the process worked in
    ///
    ///
    ///       +--------------------+
    ///       | scattering thread  |
    ///       +--------------------+
    ///                  |
    ///                  |
    ///                  V
    ///         (ratelier_scatter)
    ///         /        |       \
    ///        /         |        \
    ///   +-------+  +-------+  +-------+
    ///   |worker1|  |worker2|  |worker3|
    ///   +-------+  +-------+  +-------+
    ///        \         |        /
    ///         \        |       /
    ///          V       V      V
    ///         (ratelier_gather)
    ///                  |
    ///                  |
    ///                  V
    ///       +--------------------+
    ///       | gathering  thread  |
    ///       +--------------------+
    ///
    /// \note "ratelier" is a french word that translates here to "rack" in
    /// the meaning of the structure where a soldier puts its weapon, or if
    /// you prefer piece of metal that can hold and park a set of bicycles.



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
	/// fill the ratelier with data.
	/// Workers put objects each at a given slot and a non-worker thread (or gathering thread)
	/// get those objects in order, which releases the corresponding slots of the ratelier_gather object

    template <class T> class ratelier_gather
    {
    public:
	ratelier_gather(unsigned int size, signed int flag = 0);
	ratelier_gather(const ratelier_gather & ref) = delete;
	ratelier_gather(ratelier_gather && ref) = default;
	ratelier_gather & operator = (const ratelier_gather & ref) = delete;
	ratelier_gather & operator = (ratelier_gather && ref) noexcept = default;
	virtual ~ratelier_gather() = default;

	    /// provides to a worker thread a mean to given data with its associated index to a gathering thread

	    /// \param[in] slot is the slot number associated to the provided object "one"
	    /// \param[in] one is the object to push to the gathering thread
	    /// \param[in] flag is a purpose free signal to send to the gathering thread as associated to this object.
	    /// \note if the slot is already full an exception is thrown
	    /// \note if the ratelier_gather is full the caller will be suspended until the
	    /// non-worker thread calls gather() to make some room
	void worker_push_one(unsigned int slot, std::unique_ptr<T> & one, signed int flag = 0);

	    /// obtain the lowest continuous filled slots of the ratelier_gather and free them

	    /// \param[out] ones is a list of continuously indexed objects which immediately follows the list
	    /// provided by a previous call to gather().
	    /// \param[out] flag is the purpose free signal give by the worker and associated to each data
	void gather(std::deque<std::unique_ptr<T> > & ones, std::deque<signed int> & flag);

	    /// reset the object in its prestine state
	void reset();

    private:

	static const unsigned int cond_pending_data = 0; ///< condition when the object is empty and thread is waiting for situation change
	static const unsigned int cond_full = 1;         ///< condition when the object is full and thread is waiting for situation change

	struct slot
	{
	    std::unique_ptr<T> obj;
	    bool empty;
	    unsigned int index;
	    signed int flag;

	    slot(signed int val) { empty = true; flag = val; };
	    slot(const slot & ref) { obj.reset(); empty = ref.empty; index = ref.index; flag = ref.flag; };
	};

	unsigned int next_index; ///< next index to start the next gather() with
	std::vector<slot> table; ///< table of slots to store data
	std::map<unsigned int, unsigned int> corres; ///< associate infinite range index to index in table
	std::deque<unsigned int> empty_slot; ///< empty slot of table
	libthreadar::condition verrou;  ///< lock to manipulate private data
    };

    template <class T> ratelier_gather<T>::ratelier_gather(unsigned int size, signed int flag):
	table(size, slot(flag)),
	verrou(2)
    {
	next_index = 0;

	for(unsigned int i = 0; i < size; ++i)
	    empty_slot.push_back(i);
    }

    template <class T> void ratelier_gather<T>::worker_push_one(unsigned int slot, std::unique_ptr<T> & one, signed int flag)
    {
	verrou.lock();

	try
	{
	    while(empty_slot.empty()  // no free slot available
		  || ((empty_slot.size() == 1 && slot != next_index) // one slot available and we do not provide the lowest expecting slot num
		      && corres.begin() != corres.end() && (corres.begin())->first != next_index)) // and lowest slot is still not received
		verrou.wait(cond_full);

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
	    table[index].flag = flag;

	    empty_slot.pop_back();

	    if(verrou.get_waiting_thread_count(cond_pending_data) > 0)
		if(corres.find(next_index) != corres.end()) // some data can be gathered
		    verrou.signal(cond_pending_data); // awaking the gathering thread
	}
	catch(...)
	{
	    verrou.unlock(); // unlock first, as broadcast/signal may be the cause of the exception
	    verrou.broadcast(cond_pending_data);
	    verrou.broadcast(cond_full);
	    throw;
	}
	verrou.unlock();
    }

    template <class T> void ratelier_gather<T>::gather(std::deque<std::unique_ptr<T> > & ones, std::deque<signed int> & flag)
    {
	ones.clear();
	flag.clear();

	verrou.lock();
	try
	{
	    std::map<unsigned int, unsigned int>::iterator it;
	    std::map<unsigned int, unsigned int>::iterator tmp;

	    do
	    {
		it = corres.begin();

		while(it != corres.end())
		{
		    if(it->first > next_index) // not continuous sequence
			break; // exiting the inner while loop

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
			flag.push_back(table[it->second].flag);

			table[it->second].empty = true;
			empty_slot.push_back(it->second);
			tmp = it;
			++it;
			corres.erase(tmp);
			++next_index;
		    }
		    else // integer overload occured for the index
			++it; // skipping this entry
		}

		if(ones.empty())
		    verrou.wait(cond_pending_data);
	    }
	    while(ones.empty());

	    if(verrou.get_waiting_thread_count(cond_full) > 0)
		verrou.broadcast(cond_full); // awake all pending workers
	}
	catch(...)
	{
	    verrou.unlock(); // unlock first, as broadcast() may be the cause of the exception
	    verrou.broadcast(cond_pending_data);
	    verrou.broadcast(cond_full);
	    throw;
	}
	verrou.unlock();

	if(ones.size() != flag.size())
	    throw THREADAR_BUG;
    }

    template <class T> void ratelier_gather<T>::reset()
    {
	unsigned int size = table.size();
	next_index = 0;
	corres.clear();
	empty_slot.clear();

	for(unsigned int i = 0; i < size; ++i)
	{
	    table[i].obj.reset();
	    table[i].empty = true;
	    empty_slot.push_back(i);
	}

	verrou.lock();
	verrou.broadcast(cond_pending_data);
	verrou.broadcast(cond_full);
	verrou.unlock();
    }


} // end of namespace

#endif

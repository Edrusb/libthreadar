/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2025 Denis Corbin
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

#ifndef LIBTHREADAR_RATELIER_SCATTER_HPP
#define LIBTHREADAR_RATELIER_SCATTER_HPP

    /// \file ratelier_scatter.hpp
    /// \brief defines structure that is suitable to dispatch between many workers taking job order in consideration

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

        /// The class ratelier_scatter's purpose it to scatter an ordered set of data to many worker threads

	/// Each worker thread fetches a piece of work which is provided with an associated index. The index starts
	/// at zero and increases by one for each new piece a worker fetches to keep the sequence order of the provided
	/// data.
	///
	/// On the other end a scattering thread feeds the ratelier_scatter with data.
	///
	/// The original design for this class is to work in conjunction with a ratelier_gather structure to gather
	/// job result with their associated index, which structure will provide to a gathering thread those results in order,
	/// whatever is the effective order the workers completed their task. See ratelier_gather doc for more details.
	///
	/// The number of slot should be greater than the expected number of worker that
	/// will fetch data, for they dont stay pending for the scattering thread to refill the structure with new data.

    template <class T> class ratelier_scatter
    {
    public:
	ratelier_scatter(unsigned int size, signed int flag = 0);
	ratelier_scatter(const ratelier_scatter & ref) = delete;
	ratelier_scatter(ratelier_scatter && ref) = default;
	ratelier_scatter & operator = (const ratelier_scatter & ref) = delete;
	ratelier_scatter & operator = (ratelier_scatter && ref) noexcept = default;
	virtual ~ratelier_scatter() = default;

	    /// For the non-worker thread to provide data to the ratelier_scatter

	    /// \param one an object to scatter to workers
	    /// \param flag is a signal available to worker for any purpose it is associated
	    /// to the provided object in this call
	    /// \note the data is added to increasingly higher indexes of the virtually infinite
	    /// provided list of object, thus the index may overflow, this is not a problem unless you
	    /// expect having more data in-fly than the maximum integer that an unsigned int may carry.
	    /// Note also that the caller may be suspended if the ratelier_scatter
	    /// is full (if no worker did fetch a job).
	void scatter(std::unique_ptr<T> & one, signed int flag = 0);

	    /// For a worker thread to obtain an object in the lowest slot available

	    /// \param[out] slot the slot associated to the object obtained in return of this call
	    /// \param[out] flag a signal associated to this object by from the scattering thread
	    /// \return the next object available from the ratelier_scaller that has been given by
	    /// the non-worker thread calling the scatter() method
	    /// \note this call may suspended the caller until the scattering thread feeds
	    /// the ratelier_scatter with new data
	std::unique_ptr<T> worker_get_one(unsigned int & slot, signed int & flag);

	    /// reset the object in its prestine state

	    /// \note this resets the index to zero.
	void reset();

    private:

	static const unsigned int cond_empty = 0; ///< condition when the object is empty and thread is waiting for situation change
	static const unsigned int cond_full = 1;  ///< condition when the object is full and thread is waiting for situation change

	struct slot
	{
	    std::unique_ptr<T> obj;   ///< the object stored in this slot
	    bool empty;               ///< whether this slot is empty or not
	    unsigned int index;       ///< virtual index of the object
	    signed int flag;          ///< value of the flag signal (purpose free)

	    slot(signed int val) { empty = true; flag = val; };
	    slot(const slot & ref) { obj.reset(); empty = ref.empty; index = ref.index; flag = ref.flag; };
	};

	unsigned int next_index;       ///< index of the next slot to use (always increases but may overflood)
	unsigned int lowest_index;     ///< next index to provide to a worker
	std::vector<slot> table;       ///< table of slots to store data
	std::map<unsigned int, unsigned int> corres; ///< associate infinite range index to index in table
	std::deque<unsigned int> empty_slot;         ///< empty slot of table
	libthreadar::condition verrou;               ///< lock to manipulate private data
    };

    template <class T> ratelier_scatter<T>::ratelier_scatter(unsigned int size, signed int flag):
	table(size, slot(flag)),
	verrou(2)
    {
	next_index = 0;
	lowest_index = 0;

	for(unsigned int i = 0; i < size; ++i)
	    empty_slot.push_back(i);
    }

    template <class T> void ratelier_scatter<T>::scatter(std::unique_ptr<T> & one, signed int flag)
    {
	unsigned int tableindex;

	verrou.lock();
	try
	{
	    while(empty_slot.empty()) // ratelier_scatter is full
		verrou.wait(cond_full);

	    tableindex = empty_slot.back();

		// sanity checks

	    if(tableindex >= table.size())
		throw THREADAR_BUG;
	    if( ! table[tableindex].empty)
		throw THREADAR_BUG;

		// recording the change

	    table[tableindex].empty = false;
	    table[tableindex].obj = std::move(one);
	    table[tableindex].index = next_index;
	    table[tableindex].flag = flag;

	    corres[next_index] = tableindex;
	    ++next_index;

	    empty_slot.pop_back();
	    if(verrou.get_waiting_thread_count(cond_empty) > 0)
		verrou.signal(cond_empty);
	}
	catch(...)
	{
	    verrou.unlock();
	    verrou.broadcast(cond_empty);
	    verrou.broadcast(cond_full);
	    throw;
	}
	verrou.unlock();
    }

    template <class T> std::unique_ptr<T> ratelier_scatter<T>::worker_get_one(unsigned int & slot, signed int & flag)
    {
	std::unique_ptr<T> ret;

	verrou.lock();
	try
	{
	    std::map<unsigned int, unsigned int>::iterator it = corres.begin();
		// using sequential reading provides sorted scanning
		// of the map, looking first for the lowest index available (oldest entries)

	    do
	    {
		if(it != corres.end())
		{
		    if(it->first < lowest_index) // overflooding occured
			++it; // ignoring this slot
		    else
		    {

			    // sanity checks

			if(it->second >= table.size())
			    throw THREADAR_BUG;
			if(table[it->second].empty)
			    throw THREADAR_BUG;
			if( ! table[it->second].obj)
			    throw THREADAR_BUG;

			    // recording the change

			ret = std::move(table[it->second].obj);
			slot = table[it->second].index;
			flag = table[it->second].flag;
			table[it->second].empty = true;

			if(lowest_index != slot)
			    throw THREADAR_BUG;
			++lowest_index;

			    // reusing quicker the last block used
			    // as the back() be used first
			empty_slot.push_back(it->second);
			corres.erase(it); // removing the correspondance

			if(verrou.get_waiting_thread_count(cond_full) > 0)
			    verrou.signal(cond_full);
		    }
		}
		else
		{
			// ratelier_scatter is empty

		    verrou.wait(cond_empty);
		    it = corres.begin();
		}
	    }
	    while( ! ret);
	}
	catch(...)
	{
	    verrou.unlock();
	    verrou.broadcast(cond_empty);
	    verrou.broadcast(cond_full);
	    throw;
	}
	verrou.unlock();

	return ret;
    }

    template <class T> void ratelier_scatter<T>::reset()
    {
	unsigned int size = table.size();
	next_index = 0;
	lowest_index = 0;
	corres.clear();
	empty_slot.clear();

	for(unsigned int i = 0; i < size; ++i)
	{
	    table[i].obj.reset();
	    table[i].empty = true;
	    empty_slot.push_back(i);
	}

	verrou.lock();
	verrou.broadcast(cond_empty);
	verrou.broadcast(cond_full);
	verrou.unlock();
    }

} // end of namespace

#endif

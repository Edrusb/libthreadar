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

#ifndef LIBTHREADAR_RATELIER_SCATTER_HPP
#define LIBTHREADAR_RATELIER_SCATTER_HPP

    /// \file ratelier_scatter.hpp
    /// \brief defines structure that is suitable to dispatch between many workers taking order in consideration
    ///
    /// many worker can get each one an oject from the ratelier_scatter while a feeder thread add
    /// new one in sequence. The sequence index can be used on a ratelier_gather to gather job result in
    /// in the same order.

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
        /// the class ratelier_scatter has a fixed length range of slots of arbitrary defined object type

	/// the number of slot should be greater than the expected number of worker that
	/// will fetch data from it
	/// worker fetch an object use it and release it which once job is completed.
	/// This empties the slot ready to receive a new job. While a non-worker thread feeds
	/// the ratelier with new objects. Each object taken from the ratelier by a worker is given an index
	/// number for the worker can put this object in a given slot of a ratelier_gather object which
	/// will be in charge to deliver in sequence the different job results to a gathering thread

    template <class T> class ratelier_scatter
    {
    public:
	ratelier_scatter(unsigned int size);
	ratelier_scatter(const ratelier_scatter & ref) = delete;
	ratelier_scatter(ratelier_scatter && ref) noexcept = default;
	ratelier_scatter & operator = (const ratelier_scatter & ref) = delete;
	ratelier_scatter & operator = (ratelier_scatter && ref) noexcept = default;
	virtual ~ratelier_scatter() = default;

	    /// mean for the non-worker thread to provide data to the ratelier_scatter
	    ///
	    /// \note the data is added to new higher indexes of the virtually infininte
	    /// list of object. However the caller may be suspended if the ratelier_scatter
	    /// is full
	void scatter(std::unique_ptr<T> one);

	    /// mean for a worker thread to obtain a object in the lowest slot available
	    ///
	    /// \return the slot number that will be needed to release the slot once work is completed
	    /// \note this call may suspended the caller until the non-worker thread feeds
	    /// the ratelier_scatter with new data
	std::unique_ptr<T> worker_get_one(unsigned int & slot);

    private:
	struct slot
	{
	    std::unique_ptr<T> obj;
	    bool empty;
	    unsigned int index;

	    slot() { empty = true; };
	};

	unsigned int next_index; ///< index of the next slot to use (always increases but may overflood)
	unsigned int lowest_index; ///< next index to provide to a worker
	std::vector<slot> table; ///< table of slots to store data
	std::map<unsigned int, unsigned int> corres; ///< associate infinite range index to index in table
	std::deque<unsigned int> empty_slot; ///< empty slot of table
	libthreadar::condition verrou;  ///< lock to manipulate private data
    };

    template <class T> ratelier_scatter<T>::ratelier_scatter(unsigned int size): table(size)
    {
	next_index = 0;

	for(unsigned int i = 0; i < size; ++i)
	    empty_slot.push_back(i);
    }

    template <class T> void ratelier_scatter<T>::scatter(std::unique_ptr<T> one)
    {
	unsigned int tableindex;

	verrou.lock();
	try
	{
	    while(empty_slot.empty())
		verrou.wait();

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

	    corres[next_index] = tableindex;
	    ++next_index;

	    empty_slot.pop_back();
	    verrou.signal();
		// awake one worker thread possibily suspended if ratelier_scatter
		// was empty or had all its slot filled
		// This signal will be effective exiting this critical section
	}
	catch(...)
	{
	    verrou.broadcast();
	    verrou.unlock();
	    throw;
	}
	verrou.unlock();
    }

    template <class T> std::unique_ptr<T> ratelier_scatter<T>::worker_get_one(unsigned int & slot)
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
		    {
			++it; // ignoring this slot
			if(it == corres.end())
			    verrou.wait();
			it = corres.begin();
		    }
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
			table[it->second].empty = true;

			if(lowest_index != slot)
			    throw THREADAR_BUG;
			++lowest_index;

			    // awaking non-worker thread eventually pending for a free slot
			    // this will be done at verrou.unlock() time
			if(empty_slot.empty())
			    verrou.broadcast();

			    // reusing quicker the last block used
			    // as the back() be used first
			empty_slot.push_back(it->second);
			corres.erase(it); // removing the correspondance
		    }
		}
		else
		{
		    verrou.wait();
		    it = corres.begin();
		}
	    }
	    while( ! ret);
	}
	catch(...)
	{
	    verrou.broadcast();
	    verrou.unlock();
	    throw;
	}
	verrou.unlock();

	return ret;
    }

} // end of namespace

#endif

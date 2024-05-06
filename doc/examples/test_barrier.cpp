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

extern "C"
{
    #include <unistd.h>
}

#include "../../src/libthreadar.hpp"

using namespace std;

class myfile: public libthreadar::thread
{
public:
    myfile(libthreadar::barrier* ptr, unsigned int index): syncr(ptr), ind(index)
    {
	if(ptr == nullptr)
	    throw libthreadar::THREADAR_BUG;
    };

    ~myfile()
    {
	cancel();
	join();
    }

protected:
    virtual void inherited_run() override
    {
	cout << "thread " << ind << " starting..." << endl;
	syncr->wait();
	cout << "thread " << ind << " passed the barrier!" << endl;
    }

private:
    libthreadar::barrier* syncr;
    unsigned int ind;
};

void release_deque(deque<myfile*> & dek)
{
    deque<myfile*>::iterator it = dek.begin();

    while(it != dek.end())
    {
	if(*it != nullptr)
	    delete *it;
	++it;
    }
    dek.clear();
}

int main()
{
    unsigned int num = 10;
    libthreadar::barrier bar(num);
    deque<myfile*> file;
    myfile* ptr = nullptr;

    cout << "barrier implementation: " << libthreadar::barrier::used_implementation() << endl;

    try
    {

	for(unsigned int i = 0 ; i < num ; ++i)
	{
	    ptr = new myfile(&bar, i);
	    if(ptr == nullptr)
		release_deque(file);
	    else
		file.push_back(ptr);
	}

	for(deque<myfile*>::iterator it = file.begin(); it != file.end(); ++it)
	    (*it)->run();

	for(deque<myfile*>::iterator it = file.begin(); it != file.end(); ++it)
	    (*it)->join();
    }
    catch(...)
    {
	release_deque(file);
	throw;
    }
    release_deque(file);

    return 0;
}

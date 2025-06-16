/*********************************************************************/
// libthreadar - is a library providing several C++ classes to work with threads
// Copyright (C) 2014-2024 Denis Corbin
//
// This file is part of libthreadar package
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
#include <fcntl.h>
}

#include <iostream>
#include "../../src/libthreadar.hpp"

size_t get_default_stack_size();
void read_proc_self_maps(const std::string & context);


class my_thread: public libthreadar::thread_signal
{
protected:

    virtual void inherited_run() override
    {
	std::cout << "default thread size reported get_stacksize() from within thread: " << get_default_stack_size() << std::endl;
	read_proc_self_maps("in a thread");
    };
};

void read_proc_self_maps(const std::string & context)
{
    static const unsigned int buf_size = 1024*1024;

    int fd = open("/proc/self/maps", O_RDONLY);
    char buf[buf_size];
    int lu = 0;

    std::cout << " --------------------- " << context << "   [START]" << std::endl;
    if(fd < 0)
	return;
    try
    {
	do
	{
	    lu = read(fd, buf, buf_size);
	    if(lu > 0)
	    {
		buf[lu] = '\0';
		std::cout << buf << std::flush;
	    }
	}
	while(lu > 0);
    }
    catch(...)
    {
	close(fd);
	throw;
    }
    close(fd);
    std::cout << " --------------------- " << context << "   [END]" << std::endl;
}

size_t get_default_stack_size()
{
    size_t ret = 0;
    pthread_attr_t attr;

    switch(pthread_attr_init(&attr))
    {
    case 0:
	break;
    default:
	return 0;
    }

    pthread_attr_getstacksize(&attr,
			      &ret);

    pthread_attr_destroy(&attr);

    return ret;
}

int main()
{
    my_thread t1;

    std::cout << "default stack size is: " << get_default_stack_size() << std::endl;
    read_proc_self_maps("in main thread");

    t1.run();
    t1.join();
    t1.set_stack_size(4*1024*1024);
    t1.run();
    t1.join();
}

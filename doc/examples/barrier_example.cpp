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

#include <libthreadar/libthreadar.hpp>

barrier synchro(3); // we expect to synchronize 3 threads

void thread1::routine()
{
    synchro.wait();
	// will return only once two other threads have also called wait() on this object
}

void thread2::routine()
{
    synchro.wait();
}

void thread3::routine()
{
    synchro.wait();
	// assuming thread 3 has been executed last,
	// all three threads will now return from wait()
	// and continue in turn their execution having the
	// assurance the two other thread existed and have
	// passed this barrier at the same time
}

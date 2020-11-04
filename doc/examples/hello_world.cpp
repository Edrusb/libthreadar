// As libthreadar may not be installed on the system (this sample code
// is part of libthreader distribution) we include the libthreadar
// with explicit path. In your program you should rather use the following
// directive: #include <libthreadar/libthreadar.hpp>
#include "../../src/libthreadar.hpp"

extern "C"
{
#include <string.h>
#include <unistd.h>
} // end of extern "C"

#include <string>
#include <iostream>
using namespace std;

class my_thread: public libthreadar::thread
{
public:
	// constructor
    my_thread(const string & name): channel(10, 100) { my_name = name; counter = 0; };

	// variable my_name is accessed by both calling and inner threads but is read-only, not using mutex here
    const string & get_name() const { return my_name; };

    void send_message(const string & msg)
    {
	if(is_running())
	{
	    char *ptr;
	    unsigned int size;

	    channel.get_block_to_feed(ptr, size);
	    try
	    {
		strncpy(ptr, msg.c_str(), size);
		ptr[size - 1] = '\0';
		if(msg.size() < size)
		    size = msg.size();
	    }
	    catch(...)
	    {
		channel.feed_cancel_get_block(ptr);
		throw;
	    }
	    channel.feed(ptr, size);
	}
	else
	    cout << "Warning: inner thread is not running! Will not send a message." << endl;
    }

protected:
    void inherited_run()
    {
	char *ptr;
	unsigned int size;

	channel.reset();
	do
	{
	    channel.fetch(ptr, size);
	    try
	    {
		if(size > 1) // 1 char for the ending zero of the string
		    cout << my_name << "[" << counter++ << "] " << ptr << endl;
		else
		    cout << my_name << " received void message, ending thread normally" << endl;
	    }
	    catch(...)
	    {
		channel.fetch_push_back(ptr, size);
		throw;
	    }
	    channel.fetch_recycle(ptr);
	}
	while(size > 1);
    }

private:
    string my_name;
    unsigned int counter;
    libthreadar::fast_tampon<char> channel; // fetcher is the inner thread, feeder is the calling thread
};

int main()
{
    my_thread t1("Zorro");
    string msg;

    t1.run(); // a separated thread should run shortly

    while(! t1.is_running())
	sleep(1);

    cout << "type any string then return, the word \"stop\" ends the process" << endl;

    do
    {
	cin >> msg;
	if(msg != "stop")
	    t1.send_message(msg);
	else
	    t1.send_message("");
    }
    while(msg != "stop");

    cout << "Main thread now waits for child thread to complete" << endl;

    t1.join();

    return 0;
}

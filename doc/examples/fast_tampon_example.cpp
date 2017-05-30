
#include <libthreadar/libthreadar.hpp>


    // let's define a new thread class
    // to illustrate how to use libthreadar::fast_tampon template

class my_thread: public libthreadar::thread
{
public:
    const unsigned int block_size = 100;

	// we initialize inter as a file of 10 block
	// each of size 100 chars (T = char)
    my_thread(): inter(10, block_size) {}

	// providing a filedescriptor to read data from
    set_fd(int val)
    {
	if(is_running())
	    throw libthreadar::exception_bug(__FILE__,__LINE__);
	fd = val;
    };

	// the caller will be the fetcher of object inter
    void show()
    {
	char *ptr;
	unsigned int size;

	do
	{
	    inter.fetch(ptr, size); // we grab the next block of data
	    ptr[block_size - 1] = '\0'; // have a null terminated string
	    cout << ptr << endl;
	    inter.fetch_recycle(ptr); // as we have finished using it we recycle it into inter
	}
	while(size > 0);
	    // by convention, returning a zero sized block
	    // means that the subthread will not provide any
	    // more data

	    // so we wait for its termination
	try
	{
	    join();
	}
	catch(libthreadar::exception_range & e)
	{
	    std::cout << "Error met while reading file: " << e.get_message(": ") << std::endl;
	}
    }

protected:
	// the subthread will be the feeder of object inter
    void inherited_run()
    {
	char *ptr;
	unsigned int size, read;

	    // this sub-thread will read the data from fd up to eof or error
	do
	{
	    inter.get_block_to_feed(ptr, size); // obtaining a block of data
	    read = std::read(fd, ptr, size);    // using it
	    if(read >= 0)
		inter.feed(ptr, read);          // pushing it back to inter
	    else
	    {
		inter.feed(ptr, 0);
		throw libthreadar::exception_range(strerror(errno));
		    // this exception will be progragated to the parent thread
		    // when calling join() in the show() method above
	    }
	}
	while(read > 0);
    }

private:
    libthreadar::fast_tampon<char> inter;
    int fd;
};


int main(int argc, char *argv[])
{
    if(argc < 2)
    {
	std::cout << "usage: " << argv[0] << " <filename>" << std::endl;
	return 1;
    }

    int fd = open(argv[1]);
    if(fd >= 0)
    {
	my_thread t1;

	t1.set_fd(fd); // setup the future thread
	t1.run();      // spawn the thread that will read data from fd and pass it to inter
	t1.show();     // extract data from inter in the current (parent) thread
    }
}

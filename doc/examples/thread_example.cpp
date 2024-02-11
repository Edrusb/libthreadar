
#include <libthreadar/libthreadar.hpp>


class my_thread: public libthreadar::thread
{
public:
	// for that class we will provide
	// all parameters using set_* methods
    my_thread() { synchro.lock(); };

	// an example of thread setup, expected to
	// be called before the thread is run()
    void set_message(const std::string & msg)
    {
	if(is_running())
	    throw libthreadar::exception_bug(__FILE__,__LINE__);
	message = msg;
    };

	// we could also handle here the communication/synchronisation with
	// the thread for example using a mutex or barrier
    void unlock_thread() { synchro.unlock(); };

	// implementing our way to cleanly stop the thread
    void stop()
    {
	kill(); // requesting thread to be killed
	synchro.try_lock(); // they might be locked on synchro
	synchro.unlock();   // so we unlock them for the can end
	join(); // now waiting for their effective terminaison
	    // resetting the mutex in its initial
	    // state ready for a new run()
	synchro.try_lock();
    }

protected:
	// the method that will run as a separated thread
	// but will still have access to private fields of the
	// object. Its up to the class developper to define
	// the way the inherited_run() thread access these
	// in concurrence with the thread caller by mean
	// of provided method like unlock_thread() above
    virtual void inherited_run() override
    {
	while(1)
	{
	    synchro.lock();
	    std::cout << message << std::endl;
	}
    }

private:
    st::string message;
    libthreadar::mutex synchro;
};


main()
{
	// we will play with two sub-threads
    my_thread t1, t2;

	// we setup the object parameters
	// if more get added in the future
	// it will be easy to add a new method in the
	// class without breaking backward compatibility
    t1.set_message("hello");
    t2.set_message("world");

	// launching the threads
    t1.run();
    t2.run();

	// interacting with living threads
    for(unsigned int i = 0; i < 20; ++i)
    {
	if(i % 2 == 0)
	    t1.unlock_thread();
	if(i % 3 == 0)
	    t2.unlock_thread();
    }

	// using our own stopping method
    t1.stop();
    t2.stop();
}

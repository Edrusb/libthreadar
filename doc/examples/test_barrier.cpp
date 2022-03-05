extern "C"
{
    #include <unistd.h>
}

#include <memory>

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
	kill();
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

int main()
{
    unsigned int num = 10;
    libthreadar::barrier bar(num);
    deque<unique_ptr<myfile> > file;

    cout << "barrier implementation: " << libthreadar::barrier::used_implementation() << endl;

    for(unsigned int i = 0 ; i < num ; ++i)
	file.push_back(make_unique<myfile>(&bar, i));

    for(deque<unique_ptr<myfile> >::iterator it = file.begin(); it != file.end(); ++it)
	(*it)->run();

    for(deque<unique_ptr<myfile> >::iterator it = file.begin(); it != file.end(); ++it)
	(*it)->join();
}

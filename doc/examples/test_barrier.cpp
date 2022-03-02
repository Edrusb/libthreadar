extern "C"
{
    #include <unistd.h>
}

#include "../../src/libthreadar.hpp"

using namespace libthreadar;
using namespace std;

class myfile: public thread
{
public:
    myfile(barrier* ptr, unsigned int index): syncr(ptr), ind(index)
    {
	if(ptr == nullptr)
	    throw THREADAR_BUG;
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
    barrier* syncr;
    unsigned int ind;
};

int main()
{
    unsigned int num = 10;
    barrier bar(num);
    deque<unique_ptr<myfile> > file;

    cout << "barrier implementation: " << barrier::used_implementation() << endl;

    for(unsigned int i = 0 ; i < num ; ++i)
	file.push_back(make_unique<myfile>(&bar, i));

    for(deque<unique_ptr<myfile> >::iterator it = file.begin(); it != file.end(); ++it)
	(*it)->run();

    for(deque<unique_ptr<myfile> >::iterator it = file.begin(); it != file.end(); ++it)
	(*it)->join();
}

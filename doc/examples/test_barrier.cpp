extern "C"
{
    #include <unistd.h>
}

#include <libthreadar/libthreadar.hpp>

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
    barrier bar(num-1);
    deque<unique_ptr<myfile> > file;

    for(unsigned int i = 0 ; i < num ; ++i)
	file.push_back(make_unique<myfile>(&bar, i));

    for(deque<unique_ptr<myfile> >::iterator it = file.begin(); it != file.end(); ++it)
	(*it)->run();

    for(deque<unique_ptr<myfile> >::iterator it = file.begin(); it != file.end(); ++it)
	(*it)->join();
}

#include "stdio.h"
#include "thread/CountDownLatch.h"
#include "thread/Thread.h"

#include <functional>
#include <iostream>

using namespace std;

//test Thread

int main()
{
    auto f1 = [](){
        printf("%d\n", 555);
    };

    Thread t1(f1, "t1");
    Thread t2(f1, "t2");
    
    cout << t1.getTid() << endl;
    cout << t2.getTid() << endl;

    t1.start();
    t2.start();

    cout << t1.getTid() << " " << t1.getName() << endl;
    cout << t2.getTid() << " " << t2.getName() << endl;


    t1.join();
    t2.join();

    return 0;
}
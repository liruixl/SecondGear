#include "stdio.h"
#include "thread/CountDownLatch.h"
#include <pthread.h>
#include <iostream>

using namespace std;


// test mutex cond and countdown

void* worker(void * args)
{
    auto cd = static_cast<CountDownLatch*>(args);

    cd->wait();
    printf("%d\n", 555);
    return NULL;
}

int main()
{
    printf("%s", "hello world!\n");
    int n = 5;
    CountDownLatch countDown(5);

    pthread_t thread[5];

    for(int i = 0; i < 5; i++)
    {
        pthread_create(thread+i,NULL,worker, &countDown);
    }

    for(int i = 0; i < n; i++)
    {
        cout << "count down 1" << endl;
        countDown.countDown();
    }

    for(int i = 0; i < 5; i++)
    {
        pthread_join(thread[i], NULL);
    }

    cout << "thread join" << endl;

    return 0;
}
#include "EventLoop.h"
#include "stdio.h"
#include <unistd.h>
#include "Thread.h"

EventLoop* g_loop;

void print() 
{
  printf("2s after call me\n");
}

void threadFunc()
{
  g_loop->runAfter(2.0, print);
}

int main()
{
  EventLoop loop;
  g_loop = &loop;

  Thread t(threadFunc);
  t.start();

  loop.loop();
  return 0;
}
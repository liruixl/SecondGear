#pragma once
//#include <stdint.h>

/*
 __thread使用规则：只能修饰POD类型(类似整型指针的标量，
 不带自定义的构造、拷贝、赋值、析构的类型，
 二进制内容可以任意复制memset,memcpy,且内容可以复原)，
 不能修饰class类型，因为无法自动调用构造函数和析构函数

可以用于修饰全局变量，函数内的静态变量，
不能修饰函数的局部变量或者class的普通成员变量
*/

namespace CurrentThread
{

extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char * t_threadName;

void cacheTid();

inline int tid()
{
    //写法为：__builtin_expect(EXP, N)。
    //意思是：EXP==N的概率很大。
    if(__builtin_expect(t_cachedTid == 0, 0))
    {
        cacheTid();
    }
    return t_cachedTid;
}

inline const char* tidString()  // for logging
{
  return t_tidString;
}

inline int tidStringLength()  // for logging
{
  return t_tidStringLength;
}

inline const char* name() { return t_threadName; }

}//CurrentThread
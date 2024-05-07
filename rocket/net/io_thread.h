#ifndef ROCKET_NET_IO_THREAD_H
#define ROCKET_NET_IO_THREAD_H

#include <pthread.h>
#include <semaphore.h>
#include "rocket/net/eventLoop.h"

namespace rocket {

class IOThread {
 public:
  IOThread();
  
  ~IOThread();
  // 方便操作循环对象
  EventLoop* getEventLoop();
  // 优化，调用才开始
  void start();

  void join();

 public:
 // 必须是静态函数，线程的执行函数
  static void* Main(void* arg);


 private:
  pid_t m_thread_id {-1};    // 线程号
  pthread_t m_thread {0};   // 线程句柄-1

  EventLoop* m_event_loop {NULL}; // 当前 io 线程的 loop 对象
  // 信号量 用来做同步
  sem_t m_init_semaphore;

  sem_t m_start_semaphore;

};

}

#endif
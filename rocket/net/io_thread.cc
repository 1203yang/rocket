#include <pthread.h>
#include <assert.h>
#include "rocket/net/io_thread.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"

namespace rocket{

// 初始化信号量为0
// 创建一个线程，并在线程中执行main函数
IOThread::IOThread() {  
  // 创建一个线程并且在新的线程
  // 创建一个EventLoop

  // 初始化信号量，0表示当前进程
  int rt = sem_init(&m_init_semaphore, 0, 0);
  assert(rt == 0);

  rt = sem_init(&m_start_semaphore, 0, 0);
  assert(rt == 0);

  // 创建线程，并在线程中执行main函数
  pthread_create(&m_thread, NULL, &IOThread::Main, this);
  // wait, 直到新线程执行完 Main 函数的前置
  sem_wait(&m_init_semaphore);
  // 唤醒是通过信号量的值加一
  DEBUGLOG("IOThread [%d] create success", m_thread_id);
}

// 停止循环，释放信号量 
// 等待线程运行结束，删除循环 
IOThread::~IOThread() {

  m_event_loop->stop();
  sem_destroy(&m_init_semaphore);
  sem_destroy(&m_start_semaphore);
// 等待指定线程运行结束
  pthread_join(m_thread, NULL);
  if (m_event_loop) {
    delete m_event_loop;
    m_event_loop = NULL;
  }
}

// 在这个函数中，就是已经在新的线程里面了
// 信号量被用来做同步
// 创建循环，唤醒线程
void* IOThread::Main(void* arg) {
  // 把this对象转换成IO指针
  IOThread* thread = static_cast<IOThread*> (arg);
  // 新创建循环
  thread->m_event_loop = new EventLoop();
  thread->m_thread_id = getThreadId();

  // 唤醒等待的线程
  sem_post(&thread->m_init_semaphore);
  // 让IO 线程等待，直到我们主动的启动
  DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id);
  sem_wait(&thread->m_start_semaphore);
  DEBUGLOG("IOThread %d start loop ", thread->m_thread_id);
  thread->m_event_loop->loop();
  DEBUGLOG("IOThread %d end loop ", thread->m_thread_id);
  return NULL;

}

EventLoop* IOThread::getEventLoop() {
  return m_event_loop;
}

void IOThread::start() {
  DEBUGLOG("Now invoke IOThread %d", m_thread_id);
  sem_post(&m_start_semaphore);
  // 释放控制，信号量加一
}

// 保证主线程一直等待子线程直到退出
void IOThread::join() {
  pthread_join(m_thread, NULL);
}

}
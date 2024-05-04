#ifndef ROCKET_COMMON_MUTEX_H
#define ROCKET_COMMON_MUTEX_H

#include <pthread.h>


namespace rocket {

// 定义一个模板类
// 利用C++的特性，可以防止在忘记解锁的时候自动解锁
template <class T>
class ScopeMutex {

 public:
  // 构造函数接受一个互斥锁mutex作为参数，
  // 并在构造函数中对mutex进行加锁操作,防止死锁
  ScopeMutex(T& mutex) : m_mutex(mutex) {
    m_mutex.lock();
    m_is_lock = true;
  }

  ~ScopeMutex() {
    m_mutex.unlock();
    m_is_lock = false;
  }

  void lock() {
    if (!m_is_lock) {
      m_mutex.lock();
    }
  }

  void unlock() {
    if (m_is_lock) {
      m_mutex.unlock();
    }
  }

 private:

  T& m_mutex;
  bool m_is_lock {false};

};

// 定义互斥锁
class Mutex {
 public:
  Mutex() {
    pthread_mutex_init(&m_mutex, NULL);
  }

  ~Mutex() {
    pthread_mutex_destroy(&m_mutex);
  }

  void lock() {
    pthread_mutex_lock(&m_mutex);
  }

  void unlock() {
    pthread_mutex_unlock(&m_mutex);
  }

  pthread_mutex_t* getMutex() {
    return &m_mutex;
  }

 private:
  pthread_mutex_t m_mutex;

};

}

#endif
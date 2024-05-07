#ifndef ROCKET_NET_IO_THREAD_GROUP_H
#define ROCKET_NET_IO_THREAD_GROUP_H

#include <vector>
#include "rocket/common/log.h"
#include "rocket/net/io_thread.h"

// 本质上是一个数组

namespace rocket {

class IOThreadGroup {

 public:
 // 定义大小
  IOThreadGroup(int size);

  ~IOThreadGroup();
  // 控制线程循环的开始
  void start();

  void join();
  // 从组中获取可用的线程
  IOThread* getIOThread();

 private:

  int m_size {0}; // 数组大小
  std::vector<IOThread*> m_io_thread_groups;//数组存储线程的指针

  int m_index {0};

};

}


#endif
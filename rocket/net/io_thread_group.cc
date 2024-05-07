#include "rocket/net/io_thread_group.h"
#include "rocket/common/log.h"


namespace rocket {

// 建立一个数组
IOThreadGroup::IOThreadGroup(int size) : m_size(size) {
  m_io_thread_groups.resize(size);
  // 创建这么多IO线程
  for (size_t i = 0; (int)i < size; ++i) {
    m_io_thread_groups[i] = new IOThread();
  }
}

IOThreadGroup::~IOThreadGroup() {

}

// 调用所用IO线程的start方法
void IOThreadGroup::start() {
  for (size_t i = 0; i < m_io_thread_groups.size(); ++i) {
    m_io_thread_groups[i]->start();
  }
}

void IOThreadGroup::join() {
  for (size_t i = 0; i < m_io_thread_groups.size(); ++i) {
    m_io_thread_groups[i]->join();
  }
} 
// 获取IO线程，轮询
// 一般只在主线程调用Gruop方法
// 所以可以不考虑加锁
IOThread* IOThreadGroup::getIOThread() {
  if (m_index == (int)m_io_thread_groups.size() || m_index == -1)  {
    m_index = 0;
  }
  return m_io_thread_groups[m_index++];
}

}
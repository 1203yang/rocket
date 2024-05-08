#ifndef ROCKET_NET_FD_EVENT_GROUP_H
#define ROCKET_NET_FD_EVENT_GROUP_H

#include <vector>
#include "rocket/common/mutex.h"
#include "rocket/net/fd_event.h"

namespace rocket {
// 建立一个全局的fdevent，把所有的事件管理起来
class FdEventGroup {

 public:

  FdEventGroup(int size);

  ~FdEventGroup();
   // 获得对应fd的事件
  FdEvent* getFdEvent(int fd);

 public:
  // 全局对象，需要用静态成员函数，获得当前组的对象
  static FdEventGroup* GetFdEventGroup();

 private:
  int m_size {0};// 数组尺寸
  // 建立一个数组将所有的事件存起来
  std::vector<FdEvent*> m_fd_group;
  Mutex m_mutex;// 全局操作需要加锁

};

}

#endif
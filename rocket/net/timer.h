#ifndef ROCKET_NET_TIMER_H
#define ROCKET_NET_TIMER_H

#include <map>
#include "rocket/common/mutex.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/timer_event.h"

namespace rocket {

class Timer : public FdEvent {
 public:

  Timer();

  ~Timer();

  void addTimerEvent(TimerEvent::s_ptr event);

  void deleteTimerEvent(TimerEvent::s_ptr event);

  void onTimer(); // 当发送了 IO 事件后，eventloop 会执行这个回调函数

 private:
  void resetArriveTime();

 private:
  // 定义了一个multimap容器，Key是时间，value是指向事件的指针
  // 在容器中，按照key值排序
  std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;
  Mutex m_mutex;//可以替换成读写锁

};



}

#endif
#ifndef ROCKET_NET_TIMEREVENT
#define ROCKET_NET_TIMEREVENT

#include <functional>
#include <memory>

namespace rocket {

class TimerEvent {

 public:
 // 指向定时任务事件的智能指针
  typedef std::shared_ptr<TimerEvent> s_ptr;
  
  TimerEvent(int interval, bool is_repeated, std::function<void()> cb);

  int64_t getArriveTime() const {
    return m_arrive_time;
  }

  void setCancled(bool value) {
    m_is_cancled = value;
  }

  bool isCancled() {
    return m_is_cancled;
  }

  bool isRepeated() {
    return m_is_repeated;
  }
  // 获得回调函数
  std::function<void()> getCallBack() {
    return m_task;
  }

  void resetArriveTime();

 private:
  int64_t m_arrive_time;    // ms
  int64_t m_interval;       // ms
  bool m_is_repeated {false};
  bool m_is_cancled {false}; 

  std::function<void()> m_task;



};

}

#endif
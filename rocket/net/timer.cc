#include <sys/timerfd.h>
#include <string.h>
#include "rocket/net/timer.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"

namespace rocket {


Timer::Timer() : FdEvent() {
  // 使用的是linux自带的创建定时器的函数
  m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  DEBUGLOG("timer fd=%d", m_fd);

  // 把 fd 可读事件放到了 eventloop 上监听
  listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));
}

Timer::~Timer() {
}


void Timer::onTimer() {
  // 处理缓冲区数据，防止下一次继续触发可读事件
  // DEBUGLOG("ontimer");
  char buf[8];
  while(1) {
    if ((read(m_fd, buf, 8) == -1) && errno == EAGAIN) {
      break;
    }
  }
    // 执行定时任务
  int64_t now = getNowMs();
  std::vector<TimerEvent::s_ptr> tmps;
  // 需要执行的任务
  std::vector<std::pair<int64_t, std::function<void()>>> tasks;
  ScopeMutex<Mutex> lock(m_mutex);
  auto it = m_pending_events.begin();
  // 如果事件已经过时了，并且没有被取消
  for (it = m_pending_events.begin(); it != m_pending_events.end(); ++it) {
    if ((*it).first <= now) {
      if (!(*it).second->isCancled()) {
        tmps.push_back((*it).second);
        tasks.push_back(std::make_pair((*it).second->getArriveTime(), (*it).second->getCallBack()));
      }
    } else {
      break;
    }
  }
  // 把刚才的这些任务已经加到队列里面了，
  // 就可以删掉
  m_pending_events.erase(m_pending_events.begin(), it);
  lock.unlock();

  // 为什么要删掉再添加，因为这个map中是根据到达时间
  // 添加的，过时的任务需要重新设置时间，然后再重新
  // 进行排序

  // 需要把重复的Event 再次添加进去
  // 先遍历，看是不是重复的
  for (auto i = tmps.begin(); i != tmps.end(); ++i) {
    if ((*i)->isRepeated()) {
      // 调整 arriveTime
      (*i)->resetArriveTime();
      // 重新插入定时事件
      addTimerEvent(*i);
    }
  }

  resetArriveTime();
  // 执行回调函数，执行所有的定时任务
  for (auto i: tasks) {
    if (i.second) {
      i.second();
    }
  }
}

void Timer::resetArriveTime() {
  ScopeMutex<Mutex> lock(m_mutex);
  auto tmp = m_pending_events;//把任务取出来
  lock.unlock();
  // 如果任务队列为空，就不需要设置，
  // 那就直接退出
  if (tmp.size() == 0) {
    return;
  }
  // 获取当前时间
  int64_t now = getNowMs();
  //取第一个定时任务的时间
  auto it = tmp.begin();
  int64_t inteval = 0;
  if (it->second->getArriveTime() > now) {
    // 间隔
    inteval = it->second->getArriveTime() - now;
  } else {
    // 第一个任务已经过期了
    // 定时器马上去执行下一个任务
    inteval = 100;
  }
  
  timespec ts;
  memset(&ts, 0, sizeof(ts));
  ts.tv_sec = inteval / 1000;
  ts.tv_nsec = (inteval % 1000) * 1000000;

  itimerspec value;
  memset(&value, 0, sizeof(value));
  value.it_value = ts;
  // fd会在指定时间触发可读事件
  // EventLoop会连接到这里
  int rt = timerfd_settime(m_fd, 0, &value, NULL);
  if (rt != 0) {
    ERRORLOG("timerfd_settime error, errno=%d, error=%s", errno, strerror(errno));
  }
  DEBUGLOG("timer reset to %lld", now + inteval);

}


void Timer::addTimerEvent(TimerEvent::s_ptr event) {
  // 判断是否需要重新设置
  bool is_reset_timerfd = false;

  ScopeMutex<Mutex> lock(m_mutex);
  // 如果当前任务队列为空，需要重新设置下次时间
  // 否则事件不会触发
  if (m_pending_events.empty()) {
    is_reset_timerfd = true;
  } else {
    auto it = m_pending_events.begin();
    // 判断当前任务时间，如果比第一个任务还要早，就不会触发
    // 所以需要修改到达时间
    if ((*it).second->getArriveTime() > event->getArriveTime()) {
      is_reset_timerfd = true;
    }
  }
  m_pending_events.emplace(event->getArriveTime(), event);
  lock.unlock();

  if (is_reset_timerfd) {
    resetArriveTime();//重设时间
  }


}

void Timer::deleteTimerEvent(TimerEvent::s_ptr event) {
  event->setCancled(true);//标志位

  ScopeMutex<Mutex> lock(m_mutex);
  // 用key找到集合列表，这个列表是一个范围
  // 第一个
  auto begin = m_pending_events.lower_bound(event->getArriveTime());
  // 最后一个
  auto end = m_pending_events.upper_bound(event->getArriveTime());
  // 在这个范围里面去找event
  auto it = begin;
  for (it = begin; it != end; ++it) {
    if (it->second == event) {
      break;
    }
  }
  // 找到这个对象就删除
  if (it != end) {
    m_pending_events.erase(it);
  }
  lock.unlock();

  DEBUGLOG("success delete TimerEvent at arrive time %lld", event->getArriveTime());

}




  
}
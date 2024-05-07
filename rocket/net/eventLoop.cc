#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <string.h>
#include "rocket/common/log.h"
#include "rocket/net/eventLoop.h"
#include "rocket/common/util.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/timer.h"


// 在监听fd集合中找是否存在当前事件的Fd
// 存在就更改监听状态
// 不存在就先获得这个事件的信息结构体，
// 然后添加事件到监听集合中，并且添加到我们的属性中
#define ADD_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    int op = EPOLL_CTL_ADD; \
    if (it != m_listen_fds.end()) { \
      op = EPOLL_CTL_MOD; \
    } \
    epoll_event tmp = event->getEpollEvent(); \
    INFOLOG("epoll_event.events = %d", (int)tmp.events); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
      ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    } \
    m_listen_fds.insert(event->getFd()); \
    DEBUGLOG("add event success, fd[%d]", event->getFd()) \

#define DELETE_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    if (it == m_listen_fds.end()) { \
      return; \
    } \
    int op = EPOLL_CTL_DEL; \
    epoll_event tmp = event->getEpollEvent(); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), NULL); \
    if (rt == -1) { \
      ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    } \
    m_listen_fds.erase(event->getFd()); \
    DEBUGLOG("delete event success, fd[%d]", event->getFd()); \


namespace rocket{


// 缓存一个当前的线程局部变量
// 以确保线程只被创建一次
static thread_local EventLoop* t_current_eventloop = NULL;
static int g_epoll_max_timeout = 10000;//定时器10s
static int g_epoll_max_events = 10;

// 在线程中创建一个循环，这个循环是一个epoll实例，
// 初始化定时器和唤醒事件
EventLoop::EventLoop(){
  if (t_current_eventloop != NULL) {
    ERRORLOG("failed to create event loop, this thread has created event loop");
    exit(0);
  }
  m_thread_id = getThreadId();
  // 创建了一个 epoll 实例，并指定了它能监听的最多事件数为 10
  m_epoll_fd = epoll_create(10);

  // 返回-1是异常
  if (m_epoll_fd == -1) {
    ERRORLOG("failed to create event loop, epoll_create error, error info[%d]", errno);
    exit(0);
  }
  // // 设置非阻塞
  // m_wakeup_fd = eventfd(0,EFD_NONBLOCK);
  // if(m_wakeup_fd<0){
  //   ERRORLOG("failed to create event loop, eventfd error, error info[%d]", errno);
  //   exit(0);
  // }

  initWakeUpFdEevent();
  initTimer();

  INFOLOG("succ create event loop in thread %d", m_thread_id);
  t_current_eventloop = this;
}

// 关闭epoll实例，释放定时器和唤醒事件
EventLoop::~EventLoop(){
  close(m_epoll_fd);
  if (m_wakeup_fd_event) {
    delete m_wakeup_fd_event;
    m_wakeup_fd_event = NULL;
  }
  if (m_timer) {
    delete m_timer;
    m_timer = NULL;
  }
}
// 初始化定时器
// 创建一个定时器，并且添加到监控的EpollEvent中
void EventLoop::initTimer() {
  m_timer = new Timer();
  // 
  addEpollEvent(m_timer);
}
// 将定时任务添加到定时器中
void EventLoop::addTimerEvent(TimerEvent::s_ptr event) {
  m_timer->addTimerEvent(event);
}

// 初始化唤醒事件
// 创建一个文件描述符
// 对应的创建该唤醒事件；并绑定回调函数
// 回调函数就是读取缓冲区内容
void EventLoop::initWakeUpFdEevent() {
  // 创建一个文件描述符
  m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
  if (m_wakeup_fd < 0) {
    ERRORLOG("failed to create event loop, eventfd create error, error info[%d]", errno);
    exit(0);
  }
  INFOLOG("wakeup fd = %d", m_wakeup_fd);

  m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);

  m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this]() {
    char buf[8];
    while(read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN) {
    }
    DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
  });

  addEpollEvent(m_wakeup_fd_event);
}


// 重要的循环函数，因为服务器运行起来是不停止的
// 队列中的任务取出执行
// 然后调用epoll_wait等待事件发生，
// 并添加到任务队列中
void EventLoop::loop(){
    //服务器在运行的时候
  while(!m_stop_flag) {
    //仍然是加锁，然后取出，然后解锁
    ScopeMutex<Mutex> lock(m_mutex); 
    std::queue<std::function<void()>> tmp_tasks; 
    m_pending_tasks.swap(tmp_tasks); 
    lock.unlock();
    // 遍历任务队列，取出执行
    while (!tmp_tasks.empty()) {
      std::function<void()> cb = tmp_tasks.front();
      tmp_tasks.pop();
      if (cb) {
        cb();
      }
    }

    // 如果有定时任务需要执行，那么执行
    // 1. 怎么判断一个定时任务需要执行？ （now() > TimerEvent.arrtive_time）
    // 2. arrtive_time 如何让 eventloop 监听

    int timeout = g_epoll_max_timeout; 
    // 存放事件信息的数组
    epoll_event result_events[g_epoll_max_events];
    //DEBUGLOG("now begin to epoll_wait");
    // 在指定的 epoll 实例上等待事件的发生，一旦有事件发生或超时，
    // 就将事件信息写入到 result_events 数组中，并返回发生的事件数量
    int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout);
    //DEBUGLOG("now end epoll_wait, rt = %d", rt);
    if (rt < 0) {
      ERRORLOG("epoll_wait error, errno=%d, error=%s", errno, strerror(errno));
    } else {
      for (int i = 0; i < rt; ++i) {
        epoll_event trigger_event = result_events[i];
        FdEvent* fd_event = static_cast<FdEvent*>(trigger_event.data.ptr);
        if (fd_event == NULL) {
          ERRORLOG("fd_event = NULL, continue");
          continue;
        }
        if (trigger_event.events & EPOLLIN) { 
          //DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd())
          addTask(fd_event->handler(FdEvent::IN_EVENT));
        }
        if (trigger_event.events & EPOLLOUT) { 
          //DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd())
          addTask(fd_event->handler(FdEvent::OUT_EVENT));
        }

        // EPOLLHUP EPOLLERR
        if (trigger_event.events & EPOLLERR) {
          DEBUGLOG("fd %d trigger EPOLLERROR event", fd_event->getFd())
          // 删除出错的套接字
          deleteEpollEvent(fd_event);
          if (fd_event->handler(FdEvent::ERROR_EVENT) != nullptr) {
            DEBUGLOG("fd %d add error callback", fd_event->getFd())
            addTask(fd_event->handler(FdEvent::OUT_EVENT));
          }
        }
      }
    }
    
  }

}


//唤醒函数
void EventLoop::wakeup(){
  INFOLOG("WAKE UP");
  m_wakeup_fd_event->wakeup();
}
// 停止运行函数
void EventLoop::stop(){
    m_stop_flag = true;
    wakeup();
}
void EventLoop::dealWakeup(){

}
// 将回调函数添加到任务队列中
// 如果需要wakeup就唤醒
void EventLoop::addTask(std::function<void()> cb, bool is_wake_up /*=false*/) {
  ScopeMutex<Mutex> lock(m_mutex);
  m_pending_tasks.push(cb);
  lock.unlock();

  if (is_wake_up) {
    wakeup();
  }
}

void EventLoop::addEpollEvent(FdEvent* event) {
  if (isInLoopThread()) {
    ADD_TO_EPOLL();
  } else {
    auto cb = [this, event]() {
      ADD_TO_EPOLL();
    };
    addTask(cb, true);
  }

}
// 属于当前线程的话就删除
// 如果当前线程不在事件循环线程中，就先创建一个 Lambda 表达式作为一个任务，
// 将删除操作放入其中。然后将这个任务添加到事件循环的任务队列中。
// 这样做的目的是确保删除操作在事件循环线程中异步执行，保证了多线程环境下的安全性，
// 同时不阻塞事件循环的执行。
void EventLoop::deleteEpollEvent(FdEvent* event) {

  if (isInLoopThread()) {
    DELETE_TO_EPOLL();
  } else {
    auto cb = [this, event]() {
      DELETE_TO_EPOLL();
    };
    addTask(cb, true);
  }
}

bool EventLoop::isInLoopThread() {
  return getThreadId() == m_thread_id;
}

// 返回当前循环的对象指针
EventLoop* EventLoop::GetCurrentEventLoop() {
  if (t_current_eventloop) {
    return t_current_eventloop;
  }
  t_current_eventloop = new EventLoop();
  return t_current_eventloop;
}




}
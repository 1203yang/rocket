#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "rocket/common/mutex.h"
#include "rocket/net/wakeup_fd_event.h"

namespace rocket{

class EventLoop{
  private:
    // 由于线程是独有的，所以要记录线程号
    pid_t m_thread_id{0};
    int m_epoll_fd {0};// epoll句柄
    int m_wakeup_fd {0};
    WakeUpFdEvent*  m_wakeup_fd_event {NULL};
    bool m_stop_flag {false};// stop标志

    // 监听的fd集合
    std::set<int> m_listen_fds;
    std::queue<std::function<void()>> m_pending_tasks;

    // 加锁
    Mutex m_mutex;

    void dealWakeup();
    void initWakeUpFdEevent();
    
  public:
    EventLoop();
    ~EventLoop();
    // 重要的循环函数，因为服务器运行起来是不停止的
    void loop();
    //唤醒函数
    void wakeup();
    // 停止运行函数
    void stop();

    void addEpollEvent(FdEvent* event);

    void deleteEpollEvent(FdEvent* event);
    // 判断是否是当前线程
    bool isInLoopThread();
    // 第二个变量是否唤醒
    void addTask(std::function<void()> cb, bool is_wake_up = false);


};





}


#endif
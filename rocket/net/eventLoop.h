#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "rocket/common/mutex.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/wakeup_fd_event.h"
#include "rocket/net/timer.h"

namespace rocket{

class EventLoop{
  private:
    // 由于线程是独有的，所以要记录线程号
    pid_t m_thread_id{0};
    int m_epoll_fd {0};// 对应于epoll实例的文件描述符
    int m_wakeup_fd {0};// 唤醒的fd
    // 唤醒事件
    WakeUpFdEvent*  m_wakeup_fd_event {NULL};
    bool m_stop_flag {false};// stop标志

    // 监听的fd集合
    std::set<int> m_listen_fds;
    
    // 任务对列
    // std::function<void()> 表示一个
    // 可调用对象（callable object），它可以存储无返回值的函数（void类型的函数）
    std::queue<std::function<void()>> m_pending_tasks;

    // 锁的对象，别人想要调用也要给这个属性上锁
    // 但是这个锁锁住的时候别人没办法上锁就会阻塞
    Mutex m_mutex;
    // 定时器对象
    Timer* m_timer {NULL};
    bool m_is_looping {false};

    void dealWakeup();
    void initWakeUpFdEevent();
    void initTimer();

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
    void addTimerEvent(TimerEvent::s_ptr event);
    bool isLooping();
    static EventLoop* GetCurrentEventLoop();

};





}


#endif
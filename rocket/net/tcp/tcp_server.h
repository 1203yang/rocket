#ifndef ROCKET_NET_TCP_SERVER_H
#define ROCKET_NET_TCP_SERVER_H

#include <set>
#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/tcp/tcp_Connection.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/eventLoop.h"
#include "rocket/net/io_thread_group.h"

namespace rocket {

class TcpServer {
 public:
  TcpServer(NetAddr::s_ptr local_addr);

  ~TcpServer();

  void start();


 private:
 // 设置监听套接字的回调函数
  void init();

  // 当有新客户端连接之后需要执行
  void onAccept();

//   // 清除 closed 的连接
//   void ClearClientTimerFunc();


 private:
  // 接收对象的指针
  TcpAcceptor::s_ptr m_acceptor;
  // 网络地址的智能指针
  NetAddr::s_ptr m_local_addr;    // 本地监听地址
  // 主线程的EventLoop
  EventLoop* m_main_event_loop {NULL};    // mainReactor
  // 以及IO线程组
  IOThreadGroup* m_io_thread_group {NULL};   // subReactor 组
  
  FdEvent* m_listen_fd_event;
  // 当前连接的数量
  int m_client_counts {0};
  // 将连接保存下来

  std::set<TcpConnection::s_ptr> m_client;

//   TimerEvent::s_ptr m_clear_client_timer_event;

};

}


#endif
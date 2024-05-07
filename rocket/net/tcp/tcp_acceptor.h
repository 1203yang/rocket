#ifndef ROCKET_NET_TCP_TCP_ACCEPTOR_H
#define ROCKET_NET_TCP_TCP_ACCEPTOR_H

#include <memory>
#include "rocket/net/tcp/net_addr.h"

namespace rocket {

class TcpAcceptor {
 public:
 // 定义指向接收对象的共享指针
  typedef std::shared_ptr<TcpAcceptor> s_ptr;
  // 传入一个地址对象指针
  TcpAcceptor(NetAddr::s_ptr local_addr);

  ~TcpAcceptor();

  // std::pair<int, NetAddr::s_ptr> accept();
  int accept();
  int getListenFd();

 private:
 // 需要有ip:port，服务端监听的地址
 // bind函数绑定的地址
 // m_local_addr 是指向 NetAddr 类型对象的共享指针
  NetAddr::s_ptr m_local_addr; // 服务端监听的地址，addr -> ip:port 
// 协议
  int m_family {-1};
// 监听的套接字
  int m_listenfd {-1}; // 监听套接字

};

}

#endif
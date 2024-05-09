#ifndef ROCKET_NET_TCP_TCP_CONNECTION_H
#define ROCKET_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include <map>
#include <queue>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/io_thread.h"
#include "rocket/net/coder/abstract_coder.h"
// #include "rocket/net/rpc/rpc_dispatcher.h"

namespace rocket {
// 枚举连接状态
enum TcpState {
  NotConnected = 1,
  Connected = 2,
  HalfClosing = 3,
  Closed = 4,
};
// 用来区分客户端和服务端的connect
enum TcpConnectionType {
  TcpConnectionByServer = 1,  // 作为服务端使用，代表跟对端客户端的连接
  TcpConnectionByClient = 2,  // 作为客户端使用，代表跟对赌服务端的连接
};

class TcpConnection {
 public:
  // 智能指针
  typedef std::shared_ptr<TcpConnection> s_ptr;


 public:
  TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr,TcpConnectionType type = TcpConnectionByServer);
  TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr, TcpConnectionType type = TcpConnectionByServer);
  
  ~TcpConnection();
  // 核心方法，读
  void onRead();
  // 核心方法，执行
  void excute();
  // 核心方法，写
  void onWrite();
  // 获取设置连接状态
  void setState(const TcpState state);

  TcpState getState();
  // 清楚连接
  void clear();

  // int getFd();

  // 服务器主动关闭连接
  // 关闭占用资源的无效连接
  void shutdown();

  void setConnectionType(TcpConnectionType type);
  // 把connection监听可写事件拿出来
  // 启动监听可写事件
  void listenWrite();

  // 启动监听可读事件
  void listenRead();

  void pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

  void pushReadMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done);

  // NetAddr::s_ptr getLocalAddr();

  // NetAddr::s_ptr getPeerAddr();

  // void reply(std::vector<AbstractProtocol::s_ptr>& replay_messages);

 private:

  EventLoop* m_event_loop {NULL};   // 代表持有该连接的 IO 线程
  // 通讯所需要的两个地址，本地地址和通讯的对方地址
  NetAddr::s_ptr m_local_addr;
  NetAddr::s_ptr m_peer_addr;
  // 接收和发送的缓冲区
  TcpBuffer::s_ptr m_in_buffer;   // 接收缓冲区
  TcpBuffer::s_ptr m_out_buffer;  // 发送缓冲区
  // // 表示当前连接属于哪个线程，方便操作对应的eventloop
  // IOThread* m_io_thread {NULL};
  // tcpconnection本质上是一个socket，所以对应一个event对象
  FdEvent* m_fd_event {NULL};
  // 声明一个编程对象，要调用它的编码解码函数
  AbstractCoder* m_coder {NULL};

  TcpState m_state; // 当前连接状态

  int m_fd {0};

  TcpConnectionType m_connection_type {TcpConnectionByServer};
  // 用队列去存需要发送的信息
  // // std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>
  std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;

  // key 为 msg_id
  // 定义集合存储读到的消息
  std::map<std::string, std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;
  
};

}

#endif

#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_acceptor.h"


namespace rocket {
// 首先判断地址合法性，合法地址就设置协议、建立套接字、
// 然后设置套接字选项，允许地址重用、绑定地址、监听套接字
TcpAcceptor::TcpAcceptor(NetAddr::s_ptr local_addr) : m_local_addr(local_addr) {
  if (!local_addr->checkValid()) {
    ERRORLOG("invalid local addr %s", local_addr->toString().c_str());
    exit(0);
  }

  m_family = m_local_addr->getFamily();
  // 创建socket TCP
  m_listenfd = socket(m_family, SOCK_STREAM, 0);
 // 创建失败
  if (m_listenfd < 0) {
    ERRORLOG("invalid listenfd %d", m_listenfd);
    exit(0);
  }
  
  int val = 1;
  // 之前我们测试eventloop的时候，如果TCP使用了一个端口号，
  // 一方解除了连接就会陷入timer，所以端口号会显示被占用
  // 这里设置SO_REUSEADDR，就可以重新绑定端口
  if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
    ERRORLOG("setsockopt REUSEADDR error, errno=%d, error=%s", errno, strerror(errno));
  }
  // 绑定 将socket绑定到端口
  socklen_t len = m_local_addr->getSockLen();
  if(bind(m_listenfd, m_local_addr->getSockAddr(), len) != 0) {
    ERRORLOG("bind error, errno=%d, error=%s", errno, strerror(errno));
    exit(0);
  }
// 监听套接字，被动监听
// 第二个参数是指定待排队连接的最大数量
  if(listen(m_listenfd, 1000) != 0) {
    ERRORLOG("listen error, errno=%d, error=%s", errno, strerror(errno));
    exit(0);
  }
}

TcpAcceptor::~TcpAcceptor() {
}

int TcpAcceptor::getListenFd() {
  return m_listenfd;
}

std::pair<int, NetAddr::s_ptr> TcpAcceptor::accept() {
  if (m_family == AF_INET) {// ipv4协议
    sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t clien_addr_len = sizeof(clien_addr_len);
    // accept 函数用于接收客户端的连接请求，并返回一个新的 socket 描述符，用于与客户端进行通信。
    int client_fd = ::accept(m_listenfd, reinterpret_cast<sockaddr*>(&client_addr), &clien_addr_len);
    if (client_fd < 0) {
      ERRORLOG("accept error, errno=%d, error=%s", errno, strerror(errno));
    }
    IPNetAddr::s_ptr peer_addr = std::make_shared<IPNetAddr>(client_addr);
    INFOLOG("A client have accpeted succ, peer addr [%s]", peer_addr->toString().c_str());

    return std::make_pair(client_fd, peer_addr);
  } else {
    // ... 
    // 可以扩展其他协议
    return std::make_pair(-1, nullptr);
  }

}

}

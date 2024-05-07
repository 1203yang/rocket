#ifndef ROCKET_NET_TCP_NET_ADDR_H
#define ROCKET_NET_TCP_NET_ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>

namespace rocket {

class NetAddr {
 public:
 // 定义指向网络地址的共享指针
  typedef std::shared_ptr<NetAddr> s_ptr;
  // 返回的是通用套接字的结构体，定义为虚函数支持多态
  // 在子类实现
  virtual sockaddr* getSockAddr() = 0;
  
  virtual socklen_t getSockLen() = 0;
  // 返回协议族
  virtual int getFamily() = 0;
  // 将地址转换成字符串，比如点分十进制
  virtual std::string toString() = 0;
// 校验网络地址是否合法
  virtual bool checkValid() = 0;

};

// 定义IPV4
class IPNetAddr : public NetAddr {

 public:
  static bool CheckValid(const std::string& addr);

 public:
  // 通过ip或端口号构造
  IPNetAddr(const std::string& ip, uint16_t port);
  // 通过地址字符串构造
  IPNetAddr(const std::string& addr);
  // 通过socket结构体构造
  IPNetAddr(sockaddr_in addr);
  // 父类虚函数实现
  sockaddr* getSockAddr();
  // 
  socklen_t getSockLen();

  int getFamily();

  std::string toString();

   bool checkValid();
 
 private:
  // ip地址
  std::string m_ip;
  // 端口号
  uint16_t m_port {0};
  // 地址结构体
  sockaddr_in m_addr;

};

}

#endif

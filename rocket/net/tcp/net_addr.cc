#include <string.h>
#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"


namespace rocket {


// bool IPNetAddr::CheckValid(const std::string& addr) {
//   size_t i = addr.find_first_of(":");
//   if (i == addr.npos) {
//     return false;
//   }
//   std::string ip = addr.substr(0, i);
//   std::string port = addr.substr(i + 1, addr.size() - i - 1);
//   if (ip.empty() || port.empty()) {
//     return false;
//   }

//   int iport = std::atoi(port.c_str());
//   if (iport <= 0 || iport > 65536) {
//     return false;
//   }

//   return true;

// }


// 构造函数，初始化端口号,ip和sock_addr的结构体
IPNetAddr::IPNetAddr(const std::string& ip, uint16_t port) : m_ip(ip), m_port(port) {
  // 全部设为0
  memset(&m_addr, 0, sizeof(m_addr));
  // ipv4协议、地址传入、主机字节序->网络字节序端口号
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
  m_addr.sin_port = htons(m_port);
}
// 构造函数2，初始化端口号,ip和sock_addr的结构体
// 从字符串到结构体  
IPNetAddr::IPNetAddr(const std::string& addr) {
  size_t i = addr.find_first_of(":");
  // 没有找到就是非法地址
  if (i == addr.npos) {
    ERRORLOG("invalid ipv4 addr %s", addr.c_str());
    return;
  }
  // 在字符串中找到ip和端口号
  m_ip = addr.substr(0, i);
  m_port = std::atoi(addr.substr(i + 1, addr.size() - i - 1).c_str());
  // 初始化结构体
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
  m_addr.sin_port = htons(m_port);

}

// 构造函数3，初始化端口号
// 从结构体中获得端口号和ip 
IPNetAddr::IPNetAddr(sockaddr_in addr) : m_addr(addr) {
  // 将32位的网络字节序的IP地址转换成点分十进制的字符串表示
  // 将char*类型的字符串转换为std::string类型的字符串
  m_ip = std::string(inet_ntoa(m_addr.sin_addr));
  m_port = ntohs(m_addr.sin_port);// 网络字节序到主机字节序

}
// 获得结构体
// 将sockaddr_in转换成sockaddr
sockaddr* IPNetAddr::getSockAddr() {
  return reinterpret_cast<sockaddr*>(&m_addr);
}

socklen_t IPNetAddr::getSockLen() {
  return sizeof(m_addr);
}
// 获得协议
int IPNetAddr::getFamily() {
  return AF_INET;
}
// 获得点分十进制
std::string IPNetAddr::toString() {
  std::string re;
  re = m_ip + ":" + std::to_string(m_port);
  return re;
}

bool IPNetAddr::checkValid() {
  if (m_ip.empty()) {
    return false;  
  }

  if (m_port < 0 || m_port > 65536) {
    return false;
  }

  if (inet_addr(m_ip.c_str()) == INADDR_NONE) {
    return false;
  }
  return true;
}

}
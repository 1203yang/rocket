#ifndef ROCKET_NET_ABSTRACT_PROTOCOL_H
#define ROCKET_NET_ABSTRACT_PROTOCOL_H
#include <memory>
#include <string>

namespace rocket{
// 从class换成结构体
struct AbstractProtocol :public std::enable_shared_from_this<AbstractProtocol>{
 public:
   typedef std::shared_ptr<AbstractProtocol> s_ptr;
   
   virtual ~AbstractProtocol(){}
  public:
   // 请求号，唯一的标识rpc请求或者响应
   std::string m_msg_id;



};

}

#endif
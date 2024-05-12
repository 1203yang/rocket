#ifndef ROCKET_NET_ABSTRACT_PROTOCOL_H
#define ROCKET_NET_ABSTRACT_PROTOCOL_H
#include <memory>
#include <string>

namespace rocket{
// 从class换成结构体
struct AbstractProtocol :public std::enable_shared_from_this<AbstractProtocol>{
 public:
   typedef std::shared_ptr<AbstractProtocol> s_ptr;
   
   // 请求号，唯一的标识rpc请求或者响应
   std::string m_msg_id;
  //  std::string getMsgId(){
  //   return m_msg_id;
  //  }

  //  void setMsgId(const std::string& msg_id){
  //   m_msg_id = msg_id;
  //  }

   virtual ~AbstractProtocol(){}

};

}

#endif
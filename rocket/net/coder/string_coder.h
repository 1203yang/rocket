#ifndef ROCKERT_NET_STRING_CODER_H
#define ROCKERT_NET_STRING_CODER_H

#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/coder/abstract_protocol.h"

namespace rocket {

class StringProtocol : public AbstractProtocol {

 public:
   std::string info;

};

class StringCoder : public AbstractCoder {

  // 将 message 对象转化为字节流，写入到 buffer
  // 把 message 先转换成string的协议，再把string写到缓冲
  void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) {
    for (size_t i = 0; i < messages.size(); ++i) {
      // 智能转换，将父类的转换为子类的智能指针
      std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(messages[i]);
      out_buffer->writeToBuffer(msg->info.c_str(), msg->info.length());
    }
  }

  // 将 buffer 里面的字节流转换为 message 对象
  // 把缓冲中的数据读出，组成一个string的message对象
  // 再放到输出数组中
  void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) {
    std::vector<char> re;
    buffer->readFromBuffer(re, buffer->readAble());
    std::string info;
    for (size_t i = 0; i < re.size(); ++i) {
      info += re[i];
    }

    std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
    // msg->m_msg_id = "123456";
    msg->info = info;
    msg->m_msg_id="123456";
    out_messages.push_back(msg);

  }

};

}

#endif
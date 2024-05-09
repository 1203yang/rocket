#ifndef ROCKET_NET_CODER_TINYPB_CODER_H
#define ROCKET_NET_CODER_TINYPB_CODER_H

#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/coder/tinypb_Protocol.h"

namespace rocket {
// 主要是实现编码器和解码器
class TinyPBCoder : public AbstractCoder {

 public:

  TinyPBCoder() {}
  ~TinyPBCoder() {}

  // 将 message 对象转化为字节流，写入到 buffer
  void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer);

  // 将 buffer 里面的字节流转换为 message 对象
  void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer);


 private:
 // 将一个messages转换成字节流，返回值是字节流
  const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int& len);

};


}


#endif
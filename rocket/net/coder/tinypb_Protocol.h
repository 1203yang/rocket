#ifndef ROCKET_NET_CODER_TINYPB_PROTOCOL_H
#define ROCKET_NET_CODER_TINYPB_PROTOCOL_H 

#include <string>
#include "rocket/net/coder/abstract_protocol.h"

namespace rocket {
// 数据用结构体来存储，就不需要使用get方法
struct TinyPBProtocol : public AbstractProtocol {
 public:
  TinyPBProtocol(){}
  ~TinyPBProtocol() {}

 public:
 // 开始符和结束符,静态成员需要赋初始值
  static char PB_START;
  static char PB_END;

 public:
  int32_t m_pk_len {0};// 整包长度
  int32_t m_msg_id_len {0};// msg_id长度
  // msg_id 继承父类

  int32_t m_method_name_len {0};// 方法名字长度
  std::string m_method_name;// 方法名
  int32_t m_err_code {0};// 错误码
  int32_t m_err_info_len {0};// 错误信息长度
  std::string m_err_info;// 错误信息
  std::string m_pb_data;// 序列化长度
  int32_t m_check_sum {0};// 校验和

  bool parse_success {false};

};
// // 初始化静态成员变量
// char TinyPBProtocol::PB_START = 0x02;
// char TinyPBProtocol::PB_END = 0x03;
// 在头文件中会出现重复定义


}


#endif
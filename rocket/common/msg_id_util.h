#ifndef ROCKET_COMMON_MSGID_UTIL_H
#define ROCKET_COMMON_MSGID_UTIL_H

#include <string>

namespace rocket {
// 生成请求号，之前测试代码中都是指定的
class MsgIDUtil {

 public:
  static std::string GenMsgID();

};

}


#endif
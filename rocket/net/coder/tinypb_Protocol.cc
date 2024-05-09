#include "rocket/net/coder/tinypb_Protocol.h"

namespace rocket {
 // 初始化静态成员变量
char TinyPBProtocol::PB_START = 0x02;
char TinyPBProtocol::PB_END = 0x03;

}
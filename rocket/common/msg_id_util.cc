#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "rocket/common/msg_id_util.h"
#include "rocket/common/log.h"

namespace rocket {

// 最大的msg长度
static int g_msg_id_length = 20;
// 
static int g_random_fd = -1;
// 线程当前的msgID以及最大msgID
static thread_local std::string t_msg_id_no;
static thread_local std::string t_max_msg_id_no;

std::string MsgIDUtil::GenMsgID() {
  // 当前ID为空或者已经达到了最大
  if (t_msg_id_no.empty() || t_msg_id_no == t_max_msg_id_no) {
    if (g_random_fd == -1) {// 随机生成一个ID
      g_random_fd = open("/dev/urandom", O_RDONLY);
    }
    // 生成一个数据变量，去文件中读取
    std::string res(g_msg_id_length, 0);
    if ((read(g_random_fd, &res[0], g_msg_id_length)) != g_msg_id_length) {
      ERRORLOG("read form /dev/urandom error");
      return "";
    }// 如果文件中没有这么多，就报错
    // 如果读到了，将字符串映射到数字
    for (int i = 0; i < g_msg_id_length; ++i) {
      uint8_t x = ((uint8_t)(res[i])) % 10;
      res[i] = x + '0';
      t_max_msg_id_no += "9";
    }
    t_msg_id_no = res;
  } else {
    // 直接加1
    size_t i = t_msg_id_no.length() - 1;
    while (t_msg_id_no[i] == '9' && i >= 0) {
      i--;
    }
    if (i >= 0) {
      t_msg_id_no[i] += 1;
      for (size_t j = i + 1; j < t_msg_id_no.length(); ++j) {
        t_msg_id_no[j] = '0';
      }
    }
  }

  return t_msg_id_no;

}

}

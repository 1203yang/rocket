#include <unistd.h>
#include "rocket/net/wakeup_fd_event.h"
#include "rocket/common/log.h"


namespace rocket {
// 指定文件描述符
WakeUpFdEvent::WakeUpFdEvent(int fd) : FdEvent(fd) {
    
}

WakeUpFdEvent::~WakeUpFdEvent() {

}
// 向指定的文件描述符 m_fd 写入一个长度为 8 字节的数据，
// 来唤醒阻塞在 epoll_wait 函数中的线程或进程。

void WakeUpFdEvent::wakeup() {
  char buf[8] = {'a'};
  int rt = write(m_fd, buf, 8);
  if (rt != 8) {
    ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", m_fd);
  }
  DEBUGLOG("success read 8 bytes");
}


}
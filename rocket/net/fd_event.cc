#include "rocket/net/fd_event.h"
#include "rocket/common/log.h"

namespace rocket {

FdEvent::FdEvent(int fd) : m_fd(fd) {
  // 将监听事件信息全部设置为0
  memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::FdEvent() {
  memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::~FdEvent() {

}

// 指定调用的回调函数，也就是判断事件类型分配处理器
std::function<void()> FdEvent::handler(TriggerEvent event) {
  if (event == TriggerEvent::IN_EVENT) {
    return m_read_callback;
  } else if (event == TriggerEvent::OUT_EVENT) {
    return m_write_callback;
  } else if (event == TriggerEvent::ERROR_EVENT) {
    return m_error_callback;
  }
  return nullptr;
}

// 
void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback, std::function<void()> error_callback /*= nullptr*/) {
  if (event_type == TriggerEvent::IN_EVENT) {
    // 设置事件结构体中的事件
    m_listen_events.events |= EPOLLIN;
    m_read_callback = callback;
  } else {
    m_listen_events.events |= EPOLLOUT;
    m_write_callback = callback;
  }

  if (m_error_callback == nullptr) {
    m_error_callback = error_callback;
  } else {
    m_error_callback = nullptr;
  }

  m_listen_events.data.ptr = this;
}

}
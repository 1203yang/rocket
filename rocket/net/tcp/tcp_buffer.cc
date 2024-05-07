#include <memory>
#include <string.h>
#include "rocket/common/log.h"
#include "rocket/net/tcp/tcp_buffer.h"

namespace rocket {


// 初始化一个数组
TcpBuffer::TcpBuffer(int size) : m_size(size) {
  m_buffer.resize(size);
}

TcpBuffer::~TcpBuffer() {

}

// 返回可读字节数
int TcpBuffer::readAble() {
  return m_write_index - m_read_index;
}

// 返回可写的字节数
// 整个数组大小-写指向的下标
int TcpBuffer::writeAble() {
  return m_buffer.size() - m_write_index;
}
// 返回下标
int TcpBuffer::readIndex() {
  return m_read_index;
}

int TcpBuffer::writeIndex() {
  return m_write_index;
}
// 写入缓冲区
// 先判断大小，如果小就扩容，
void TcpBuffer::writeToBuffer(const char* buf, int size) {
  if (size > writeAble()) {
    // 调整 buffer 的大小，扩容
    int new_size = (int)(1.5 * (m_write_index + size));
    resizeBuffer(new_size);
  }
  // 将写入的东西拷贝到缓冲区中
  memcpy(&m_buffer[m_write_index], buf, size);
  m_write_index += size; // 更新下标
}

// 从缓冲区中读
// 要先判断可读的字节数
// 
void TcpBuffer::readFromBuffer(std::vector<char>& re, int size) {
  if (readAble() == 0) {
    return;
  }
  // 取小的那个
  int read_size = readAble() > size ? size : readAble();
  // 建立一个临时数组
  std::vector<char> tmp(read_size);
  memcpy(&tmp[0], &m_buffer[m_read_index], read_size);

  re.swap(tmp); 
  // 更新下标
  m_read_index += read_size;

  adjustBuffer();
}
// 更改缓冲区尺寸
// 
void TcpBuffer::resizeBuffer(int new_size) {
  std::vector<char> tmp(new_size); 
  // 将原来的数组中的东西拷贝过来
  int count = std::min(new_size, readAble());
  // 将源地址buffer开始的count个
  // 字节复制到目标地址tmp处，返回指向目标地址tmp的指针。
  memcpy(&tmp[0], &m_buffer[m_read_index], count);
  m_buffer.swap(tmp);// 交换
  // 更新两个下标
  m_read_index = 0;
  m_write_index = m_read_index + count;

}

// 调整要先设定一个条件，什么时候要调整
// 满足条件就拷贝、交换
// 什么时候调用：1从缓冲区读完之后
// 2 移动下标的两种情况
void TcpBuffer::adjustBuffer() {
  if (m_read_index < int(m_buffer.size() / 3)) {
    return;
  }
  // 临时对象拷贝
  std::vector<char> buffer(m_buffer.size());
  int count = readAble();
  // 将还没读的拷贝到临时缓冲区
  memcpy(&buffer[0], &m_buffer[m_read_index], count);
  m_buffer.swap(buffer);
  // 更新下标
  m_read_index = 0;
  m_write_index = m_read_index + count;
  // 将临时的清除
  buffer.clear();
}

// 向右移动读，所以要判断不能大于缓冲区尺寸
// 可以的话就移动
void TcpBuffer::moveReadIndex(int size) {
  size_t j = m_read_index + size;
  if (j >= m_buffer.size()) {
    ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_buffer.size());
    return;
  }
  m_read_index = j;
  adjustBuffer();
}

// 向右移动写，所以要判断不能大于缓冲区尺寸
// 可以的话就移动
void TcpBuffer::moveWriteIndex(int size) {
  size_t j = m_write_index + size;
  if (j >= m_buffer.size()) {
    ERRORLOG("moveWriteIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_buffer.size());
    return;
  }
  m_write_index = j;
  adjustBuffer();

}


}
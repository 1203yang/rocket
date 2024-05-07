#ifndef ROCKET_NET_TCP_TCP_BUFFER_H
#define ROCKET_NET_TCP_TCP_BUFFER_H

#include <vector>
#include <memory>

namespace rocket {

class TcpBuffer {

 public:

  typedef std::shared_ptr<TcpBuffer> s_ptr;

  TcpBuffer(int size);

  ~TcpBuffer();

  // 返回可读字节数
  int readAble();

  // 返回可写的字节数
  int writeAble();
  // 获取指针指向的位置
  int readIndex();
  int writeIndex();
  // 写入数据及大小
  void writeToBuffer(const char* buf, int size);
  // 读取数据，将读到的写到容器中
  void readFromBuffer(std::vector<char>& re, int size);
  // 更改大小
  void resizeBuffer(int new_size);
  // 调整回收
  void adjustBuffer();
  // 手动调整下标
  void moveReadIndex(int size);

  void moveWriteIndex(int size);

 private:
  // 读指针
  int m_read_index {0};
  // 写指针
  int m_write_index {0};
  // 缓冲区大小
  int m_size {0};

 public:
 // 缓冲区数组
  std::vector<char> m_buffer;

};


}


#endif
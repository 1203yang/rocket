#ifndef ROCKET_COMMON_UTIL_H
#define ROCKET_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>

namespace rocket {
// 进程号
pid_t getPid();
// 线程号
pid_t getThreadId();

int64_t getNowMs();
// 从网络字节序读in32的数
int32_t getInt32FromNetByte(const char* buf);

}

#endif
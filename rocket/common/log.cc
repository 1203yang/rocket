#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "rocket/common/config.h"




namespace rocket{


  // 定义了一个静态指针变量g_logger
  // 用于保存全局的Logger对象指针
  static Logger* g_logger = NULL;
  // 获取全局的Logger对象指针g_logger
  // 确保只有一个全局的Logger对象

  Logger* Logger::GetGlobalLogger() {
    if(g_logger){
      return g_logger;
    }
  
  }
  // 从配置中读取日志级别，转换成LogLevel级别，
  // 初始化全局的日志级别
  void Logger::InitGlobalLogger(){
    LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
    // 获取字符串对象（如 std::string）中存储的 C 风格字符串（以 null 终止的字符数组）的方法
    printf("Init log level [%s]\n",LogLevelToString(global_log_level).c_str());
    g_logger = new Logger(global_log_level);
  }


  // 将LogLevel转换为字符串函数实现
  std::string LogLevelToString(LogLevel level) {
    switch (level) {
      case Debug:
        return "DEBUG";

      case Info:
        return "INFO";

      case Error:
        return "ERROR";
      default:
        return "UNKNOWN";
      }
  }
  LogLevel StringToLogLevel(const std::string& log_level) {
  if (log_level == "DEBUG") {
    return Debug;
  } else if (log_level == "INFO") {
    return Info;
  } else if (log_level == "ERROR") {
    return Error;
  } else {
    return Unkonwn;
  }
}

  // 打印日志
  // 返回的是包含时间、进程线程号的字符串
  std::string LogEvent::toString(){
    struct timeval now_time;
    // 获取当前时间，包括秒数和微秒数，
    // 第一个参数是用来保存时间的指针，第二个参数保存时区
    gettimeofday(&now_time,nullptr);

    struct tm now_time_t;
    // 将秒数转换为当地时间
    // 当前时间秒数，保存转换后时间的结构体
    localtime_r(&(now_time.tv_sec),&now_time_t);

    char buf[128];
    // 将时间结构以指定格式转换为字符串并保存在指定的数组中
    // 保存转换后的字符串的数组,数组大小，格式，需要转换的时间结构
    strftime(&buf[0],128,"%y-%m-%d %H:%M:%S",&now_time_t);
    std::string time_str(buf);
    // 将时间字符串buf加上微秒数ms，最终得到包含毫秒的时间字符串time_str
    int ms = now_time.tv_usec/1000;
    time_str = time_str + "." + std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();

    // 用于创建一个可以进行字符串流操作的对象
    std::stringstream ss;
    // 字符串的拼接

    ss << "[" << LogLevelToString(m_level) << "]\t"
      << "[" << time_str << "]\t"
      << "[" << m_pid << ":" << m_thread_id << "]\t";
      
    
    return ss.str();

  }
  // 将信息推送到缓冲区
  void Logger::pushLog(const std::string& msg) {
    //ScopeMutex 是一个模板类，
    //通过模板参数 Mutex 指定了具体的互斥锁类型，即 m_mutex 是一个互斥锁对象。
    //并在初始化 ScopeMutex<Mutex> 对象时将互斥锁对象传入，
    // 然后在 ScopeMutex 的构造函数中将该互斥锁对象上锁。
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.push(msg);
    lock.unlock();
  }
  // 取出缓冲区的数据，并且打印
  void Logger::log(){
    // 更安全的做法是把buffer取出来
    ScopeMutex<Mutex> lock(m_mutex);
    std::queue<std::string> tmp ;
    m_buffer.swap(tmp);
    // 取出buffer中内容之后解锁
    lock.unlock();

    while(!tmp.empty()){
      std::string msg = tmp.front();
      tmp.pop();
      printf(msg.c_str());
    }
    
  }
}
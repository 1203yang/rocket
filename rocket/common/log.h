#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <semaphore.h>

#include "rocket/common/config.h"
#include "rocket/common/mutex.h"


namespace rocket{





// 定义一个模板函数 将格式化字符串和参数一起转换成字符串。
// 函数接受一个格式化字符串str和一个可不变数量的参数args并返回格式化后的字符串
template<typename... Args>
std::string formatString(const char* str, Args&&... args) {
  // 通过调用snprintf函数获取格式化后的字符串长度
  int size = snprintf(nullptr, 0, str, args...);
  std::string result;
  if (size > 0) {
    // 调整字符串的大小以存储格式化后的字符串
    result.resize(size);
    // 指向输出字符串的指针，字符串的大小，格式化字符串，可变参数
    snprintf(&result[0], size + 1, str, args...);
  }
  return result;
}
// 定义宏
#define DEBUGLOG(str, ...) \
  if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Debug)\
  {\
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Debug))->toString()\
     +  "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str,##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log();                                                                               \
  }\

#define INFOLOG(str, ...) \
  if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Info)\
  {\
     rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Info))->toString()\
     +  "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str,##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log();                                                                                \
  }\

#define ERRORLOG(str, ...) \
  if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Error)\
  {\
     rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Error))->toString()\
     +  "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str,##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log();                                                                                \
  }\


    // 日志级别的类
    enum LogLevel{
        Unkonwn = 0,
        Debug = 1,
        Info = 2,
        Error = 3
    };

    // 需要将LogLevel转换为字符串
    std::string LogLevelToString(LogLevel level);
    // 把字符串变成int
    LogLevel StringToLogLevel(const std::string& log_level);


    // 日志器
    // 1.提供打印日志的方法
    // 2.设置日志输出的路径
    class Logger{
      public:
        // 定义了一个名为s_ptr的别名，
        //用于表示指向Logger对象的std::shared_ptr类型的智能指针。
        typedef std::shared_ptr<Logger> s_ptr;
        //Logger(LogLevel level, int type = 1);
        Logger(LogLevel level):m_set_level(level){}
        // 打印日志时间的方法
        void pushLog(const std::string& msg);
        
        //void pushAppLog(const std::string& msg);

        //void init();

        void log();

        LogLevel getLogLevel()const{
          return m_set_level;
        }
        

      public:
        // 获取全局Logger对象的实例
        static Logger* GetGlobalLogger();
        static void InitGlobalLogger();
        //static void InitGlobalLogger(int type = 1);

      private:
        LogLevel m_set_level;
        std::queue<std::string> m_buffer;
        // 创建互斥锁对象
        Mutex m_mutex;
    };



    // 日志事件的类，规定日志包含什么以及格式
    class LogEvent{
     public:
       // 构造函数
       LogEvent(LogLevel level) : m_level(level) {}
       // 获得文件名称
       std::string getFilename() const{
        return m_file_name;
       }
       LogLevel getLogLevel() const{
        return m_level;
       }
       // 打印日志的函数
       std::string toString();
     
     private:
       std::string m_file_name; // 文件名
       int32_t m_file_line; // 行号
       int32_t m_pid; //进程号
       int32_t m_thread_id; //线程号

       LogLevel m_level; // 日志级别

    };
}

#endif
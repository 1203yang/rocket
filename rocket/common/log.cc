#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "rocket/common/config.h"
#include "rocket/net/eventLoop.h"
#include "rocket/common/run_Time.h"




namespace rocket{


// 定义了一个静态指针变量g_logger
// 用于保存全局的Logger对象指针
static Logger* g_logger = NULL;
// 获取全局的Logger对象指针g_logger
// 确保只有一个全局的Logger对象

void CoredumpHandler(int signal_no) {
  ERRORLOG("progress received invalid signal, will exit");
  g_logger->flush();
  pthread_join(g_logger->getAsyncLopger()->m_thread, NULL);
  pthread_join(g_logger->getAsyncAppLopger()->m_thread, NULL);

  signal(signal_no, SIG_DFL);
  raise(signal_no);
}


Logger* Logger::GetGlobalLogger() {
  return g_logger;
}

Logger::Logger(LogLevel level, int type /*=1*/) : m_set_level(level), m_type(type) {

  if (m_type == 0) {
    return;
  }
  m_asnyc_logger = std::make_shared<AsyncLogger>(
      Config::GetGlobalConfig()->m_log_file_name + "_rpc",
      Config::GetGlobalConfig()->m_log_file_path,
      Config::GetGlobalConfig()->m_log_max_file_size);
  
  m_asnyc_app_logger = std::make_shared<AsyncLogger>(
      Config::GetGlobalConfig()->m_log_file_name + "_app",
      Config::GetGlobalConfig()->m_log_file_path,
      Config::GetGlobalConfig()->m_log_max_file_size);
}

void Logger::flush() {
  syncLoop();
  m_asnyc_logger->stop();
  m_asnyc_logger->flush();

  m_asnyc_app_logger->stop();
  m_asnyc_app_logger->flush();
}


void Logger::init() {
  if (m_type == 0) {
    return;
  }
  m_timer_event = std::make_shared<TimerEvent>(Config::GetGlobalConfig()->m_log_sync_inteval, true, std::bind(&Logger::syncLoop, this));
  EventLoop::GetCurrentEventLoop()->addTimerEvent(m_timer_event);
  signal(SIGSEGV, CoredumpHandler);
  signal(SIGABRT, CoredumpHandler);
  signal(SIGTERM, CoredumpHandler);
  signal(SIGKILL, CoredumpHandler);
  signal(SIGINT, CoredumpHandler);
  signal(SIGSTKFLT, CoredumpHandler);

}


void Logger::syncLoop() {
  // 同步 m_buffer 到 async_logger 的buffer队尾
  // printf("sync to async logger\n");
  std::vector<std::string> tmp_vec;
  ScopeMutex<Mutex> lock(m_mutex);
  tmp_vec.swap(m_buffer);
  lock.unlock();

  if (!tmp_vec.empty()) {
    m_asnyc_logger->pushLogBuffer(tmp_vec);
  }
  tmp_vec.clear();

  // 同步 m_app_buffer 到 app_async_logger 的buffer队尾
  std::vector<std::string> tmp_vec2;
  ScopeMutex<Mutex> lock2(m_app_mutex);
  tmp_vec2.swap(m_app_buffer);
  lock.unlock();

  if (!tmp_vec2.empty()) {
    m_asnyc_app_logger->pushLogBuffer(tmp_vec2);
  }

}

// 从配置中读取日志级别，转换成LogLevel级别，
// 初始化全局的日志级别
void Logger::InitGlobalLogger(int type /*=1*/){
  LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
  // 获取字符串对象（如 std::string）中存储的 C 风格字符串（以 null 终止的字符数组）的方法
  printf("Init log level [%s]\n",LogLevelToString(global_log_level).c_str());
  g_logger = new Logger(global_log_level);
  g_logger->init();
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
    
  // 获取当前线程处理的请求的 msgid

  std::string msgid = RunTime::GetRunTime()->m_msgid;
  std::string method_name = RunTime::GetRunTime()->m_method_name;
  if (!msgid.empty()) {
    ss << "[" << msgid << "]\t";
  }

  if (!method_name.empty()) {
    ss << "[" << method_name << "]\t";
  }
  return ss.str();

}
// 将信息推送到缓冲区
void Logger::pushLog(const std::string& msg) {
  //ScopeMutex 是一个模板类，
  //通过模板参数 Mutex 指定了具体的互斥锁类型，即 m_mutex 是一个互斥锁对象。
  //并在初始化 ScopeMutex<Mutex> 对象时将互斥锁对象传入，
  // 然后在 ScopeMutex 的构造函数中将该互斥锁对象上锁。
  if (m_type == 0) {
    printf((msg + "\n").c_str());
    return;
  }
  ScopeMutex<Mutex> lock(m_mutex);
  m_buffer.push_back(msg);
  lock.unlock();
}

void Logger::pushAppLog(const std::string& msg) {
  ScopeMutex<Mutex> lock(m_app_mutex);
  m_app_buffer.push_back(msg);
  lock.unlock();
}

// 取出缓冲区的数据，并且打印
void Logger::log(){
  // // 更安全的做法是把buffer取出来
  // ScopeMutex<Mutex> lock(m_mutex);
  // std::queue<std::string> tmp ;
  // m_buffer.swap(tmp);
  // // 取出buffer中内容之后解锁
  // lock.unlock();

  // while(!tmp.empty()){
  //   std::string msg = tmp.front();
  //   tmp.pop();
  //   printf(msg.c_str());
  // }
}

AsyncLogger::AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size) 
  : m_file_name(file_name), m_file_path(file_path), m_max_file_size(max_size) {
  
  sem_init(&m_sempahore, 0, 0);

  assert(pthread_create(&m_thread, NULL, &AsyncLogger::Loop, this) == 0);

  // assert(pthread_cond_init(&m_condtion, NULL) == 0);

  sem_wait(&m_sempahore);

}


void* AsyncLogger::Loop(void* arg) {
  // 将 buffer 里面的全部数据打印到文件中，然后线程睡眠，直到有新的数据再重复这个过程

  AsyncLogger* logger = reinterpret_cast<AsyncLogger*>(arg); 

  assert(pthread_cond_init(&logger->m_condtion, NULL) == 0);

  sem_post(&logger->m_sempahore);

  while(1) {
    ScopeMutex<Mutex> lock(logger->m_mutex);
    while(logger->m_buffer.empty()) {
      // printf("begin pthread_cond_wait back \n");
      pthread_cond_wait(&(logger->m_condtion), logger->m_mutex.getMutex());
    }
    // printf("pthread_cond_wait back \n");

    std::vector<std::string> tmp;
    tmp.swap(logger->m_buffer.front());
    logger->m_buffer.pop();

    lock.unlock();

    timeval now;
    gettimeofday(&now, NULL);

    struct tm now_time;
    localtime_r(&(now.tv_sec), &now_time);
    
    const char* format = "%Y%m%d";
    char date[32];
    strftime(date, sizeof(date), format, &now_time);

    if (std::string(date) != logger->m_date) {
      logger->m_no = 0;
      logger->m_reopen_flag = true;
      logger->m_date = std::string(date);
    }
    if (logger->m_file_hanlder == NULL) {
      logger->m_reopen_flag = true;
    }

    std::stringstream ss;
    ss << logger->m_file_path << logger->m_file_name << "_"
      << std::string(date) << "_log.";
    std::string log_file_name = ss.str() + std::to_string(logger->m_no);

    if (logger->m_reopen_flag) {
      if (logger->m_file_hanlder) {
        fclose(logger->m_file_hanlder);
      }
      logger->m_file_hanlder = fopen(log_file_name.c_str(), "a");
      logger->m_reopen_flag = false;
    }

    if (ftell(logger->m_file_hanlder) > logger->m_max_file_size) {
      fclose(logger->m_file_hanlder);

      log_file_name = ss.str() + std::to_string(logger->m_no++);
      logger->m_file_hanlder = fopen(log_file_name.c_str(), "a");
      logger->m_reopen_flag = false;

    }

    for (auto& i : tmp) {
      if (!i.empty()) {
        fwrite(i.c_str(), 1, i.length(), logger->m_file_hanlder);
      }
    }
    fflush(logger->m_file_hanlder);

    if (logger->m_stop_flag) {
      return NULL;
    }
  }

  return NULL;
}


void AsyncLogger::stop() {
  m_stop_flag = true;
}

void AsyncLogger::flush() {
  if (m_file_hanlder) {
    fflush(m_file_hanlder);
  }
}

void AsyncLogger::pushLogBuffer(std::vector<std::string>& vec) {
  ScopeMutex<Mutex> lock(m_mutex);
  m_buffer.push(vec);
  pthread_cond_signal(&m_condtion);

  lock.unlock();

  // 这时候需要唤醒异步日志线程
  // printf("pthread_cond_signal\n");
}

}
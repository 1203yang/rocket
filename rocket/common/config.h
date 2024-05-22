#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

#include <map>
#include <tinyxml/tinyxml.h>
#include <string>
#include "rocket/net/tcp/net_addr.h"

namespace rocket{

struct RpcStub {
  std::string name;
  NetAddr::s_ptr addr;
  int timeout {2000};
};
 
 class Config{
   public:
    // 构造函数，传入的是xml文件地址
    Config(const char* xmlfile);
    Config();
   ~Config();

    // 获得以及设置全局的配置
    static Config* GetGlobalConfig();
    static void SetGlobalConfig(const char* xmlfile);

    // 日志级别
    std::string m_log_level;
    std::string m_log_file_name;
    std::string m_log_file_path;
    int m_log_max_file_size {0};
    int m_log_sync_inteval {0};   // 日志同步间隔，ms

    int m_port {0};
    int m_io_threads {0};

    TiXmlDocument* m_xml_document{NULL};

    std::map<std::string, RpcStub> m_rpc_stubs;

 };

 }


#endif
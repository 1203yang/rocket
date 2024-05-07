#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

#include <map>
#include <tinyxml/tinyxml.h>
#include <string>

 namespace rocket{
 
 class Config{
   public:
    // 构造函数，传入的是xml文件地址
    Config(const char* xmlfile);
    //Config();
   // ~Config();

    // 获得以及设置全局的配置
    static Config* GetGlobalConfig();
    static void SetGlobalConfig(const char* xmlfile);

    // 日志级别
    std::string m_log_level;

 };

 }


#endif
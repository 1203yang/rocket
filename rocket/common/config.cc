#include <tinyxml/tinyxml.h>
#include "rocket/common/config.h"

// 因为要从xml文件中读取根节点和日志结点，
// 所以可以封装成一个宏
// 如果读到的结点为空，就把结点的名字打印出来
#define READ_XML_NODE(name, parent) \
  TiXmlElement* name##_node = parent->FirstChildElement(#name); \
  if (!name##_node) { \
    printf("Start rocket server error, failed to read node [%s]\n", #name); \
    exit(0); \
  } \

// 从结点中获取字符串
// 如果结点为空或者字符串为空就打印错误信息
// 否则就把得到的字符串赋给str
#define READ_STR_FROM_XML_NODE(name, parent) \
  TiXmlElement* name##_node = parent->FirstChildElement(#name); \
  if (!name##_node|| !name##_node->GetText()) { \
    printf("Start rocket server error, failed to read config file %s\n", #name); \
    exit(0); \
  } \
  std::string name##_str = std::string(name##_node->GetText()); \



namespace rocket{

static Config* g_config = NULL;

Config* Config::GetGlobalConfig() {
  return g_config;
}
// 设置配置
// 如果没有配置过，并且xml文件非空就设置
void Config::SetGlobalConfig(const char* xmlfile) {
  if (g_config == NULL) {
    if (xmlfile != NULL) {
      g_config = new Config(xmlfile);
    } 
    
  }
}

// 构造函数中
Config::Config(const char* xmlfile){
    TiXmlDocument* xml_document = new TiXmlDocument();
    // 从xml文件中加载配置文件
    bool rt = xml_document->LoadFile(xmlfile);
    // 如果加载失败就在控制台打印信息
    if (!rt) {
      printf("Start rocket server error, failed to read config file %s,error info[%s]\n", xmlfile,xml_document->ErrorDesc());
      exit(0);
    }


    // 遍历结点，首先要从文件中获取根节点
    READ_XML_NODE(root, xml_document);
    // 从根节点中获取日志结点
    READ_XML_NODE(log, root_node);
    //READ_XML_NODE(server, root_node);

    // 从日志结点中读取日志级别的字符串
    READ_STR_FROM_XML_NODE(log_level, log_node);
    m_log_level = log_level_str;

}

}
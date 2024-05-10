#ifndef ROCKET_NET_RPC_RPC_DISPATCHER_H
#define ROCKET_NET_RPC_RPC_DISPATCHER_H

#include <map>
#include <memory>
#include <google/protobuf/service.h>

#include "rocket/net/coder/abstract_protocol.h"
#include "rocket/net/coder/tinypb_Protocol.h"

namespace rocket {

class TcpConnection;// 前提声明

class RpcDispatcher {

 public:

  static RpcDispatcher* GetRpcDispatcher();

 public:
  // 指向服务端的智能指针
  typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;
  // 最主要的方法
  // 根据一个请求，调用对应的rpc方法，得到一个响应的message对象
  void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection* connection);
  // 注册的方法
  void registerService(service_s_ptr service);
  // 设置错误码
  void setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string err_info);

 private:
 // 方法名OrderService.make_order ，要从中解析出服务和方法
  bool parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name);

 private:
 // 存储注册的service对象，智能指针
  std::map<std::string, service_s_ptr> m_service_map;
};


}

#endif
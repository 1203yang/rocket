#ifndef ROCKER_NET_RPC_RPC_CONTROLLER_H
#define ROCKER_NET_RPC_RPC_CONTROLLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <string>

#include "rocket/net/tcp/net_addr.h"
#include "rocket/common/log.h"


namespace rocket {
// 相当于rpc的配置文件
class RpcController : public google::protobuf::RpcController {

 public:
  RpcController() { INFOLOG("RpcController"); } 
  ~RpcController() { INFOLOG("~RpcController"); } 

  void Reset();

  bool Failed() const;

  std::string ErrorText() const;

  void StartCancel();

  void SetFailed(const std::string& reason);

  bool IsCanceled() const;

  void NotifyOnCancel(google::protobuf::Closure* callback);
  // 上面为继承父类的函数
  // 设置错误信息
  void SetError(int32_t error_code, const std::string error_info);

  int32_t GetErrorCode();

  std::string GetErrorInfo();

  void SetMsgId(const std::string& msg_id);

   std::string GetMsgId();

  void SetLocalAddr(NetAddr::s_ptr addr);

  void SetPeerAddr(NetAddr::s_ptr addr);

  NetAddr::s_ptr GetLocalAddr();

  NetAddr::s_ptr GetPeerAddr();

  void SetTimeout(int timeout);

  int GetTimeout();

//   bool Finished();

//   void SetFinished(bool value);
 
 private:
  int32_t m_error_code {0}; // 错误码
  std::string m_error_info; // 错误信息
  std::string m_msg_id;     // 请求id

  bool m_is_failed {false}; //rpc是否调用失败
  bool m_is_cancled {false};// rpc是否被取消
  bool m_is_finished {false};
  // 本地地址和对方地址
  NetAddr::s_ptr m_local_addr;
  NetAddr::s_ptr m_peer_addr;
  // 超时时间
  int m_timeout {1000};   // ms

};

}


#endif
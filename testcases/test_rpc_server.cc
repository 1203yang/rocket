#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include <google/protobuf/service.h>
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/common/log.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/coder/string_coder.h"
#include "rocket/net/coder/abstract_protocol.h"
#include "rocket/net/coder/tinypb_Coder.h"
#include "rocket/net/coder/tinypb_Protocol.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include "rocket/net/rpc/rpc_dispatcher.h"

#include "order.pb.h"
// 定义一个类，继承order方法，并实现虚函数
// 下单的类
class OrderImpl : public Order {
 public:
  void makeOrder(google::protobuf::RpcController* controller,
                      const ::makeOrderRequest* request,
                      ::makeOrderResponse* response,
                      ::google::protobuf::Closure* done) {
    APPDEBUGLOG("start sleep 5s");
    sleep(5);
    APPDEBUGLOG("end sleep 5s");
    if (request->price() < 10) {
      response->set_ret_code(-1);// 钱不够
      response->set_res_info("short balance");
      return;
    }// 下单id
    response->set_order_id("20230514");
    APPDEBUGLOG("call makeOrder success");
    if (done) {
      done->Run();
      delete done;
      done = NULL;
    }
  }
};
void test_tcp_server(){
  rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12346);
  DEBUGLOG("create addr %s",addr->toString().c_str());
  rocket::TcpServer tcp_server(addr);
  tcp_server.start();
}


int main(int argc, char* argv[]) {

  if (argc != 2) {
    printf("Start test_rpc_server error, argc not 2 \n");
    printf("Start like this: \n");
    printf("./test_rpc_server ../conf/rocket.xml \n");
    return 0;
  }

  rocket::Config::SetGlobalConfig("../conf/rocket.xml");

  rocket::Logger::InitGlobalLogger();

  std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
     // 启动之前注册好service
  rocket::RpcDispatcher::GetRpcDispatcher()->registerService(service);

  rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", rocket::Config::GetGlobalConfig()->m_port);
  rocket::TcpServer tcp_server(addr);
  tcp_server.start();
  // test_tcp_server();

  return 0;
}
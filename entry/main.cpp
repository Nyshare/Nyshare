#include "HttpHandler.h"
#include "Server.h"

void onconnect(Connection *conn);

int main() {
  Server::instance().set_handle_onconnect_function(onconnect);
  Server::instance().run();
  return 0;
}

void onconnect(Connection *conn) {
  conn->read();
  if (conn->state() == Connection::DISCONNECT) {
    return;
  }
  // 处理数据
  conn->write(HttpHandler().handleHttpRequest(conn->read_buffer()));
}

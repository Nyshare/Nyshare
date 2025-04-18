#pragma once

#include <iostream>
#include <unordered_map>

#include "Acceptor.h"
#include "Connection.h"
#include "Epoll.h"

class Server {
 public:
  static Server &instance();
  void run();

  void set_handle_onconnect_function(
      const std::function<void(Connection *)> &function);

 private:
  Server();

  void new_connection(int fd, const std::string &host, int port);

  void handle_disconnect(int fd);

  void load_config();

 private:
  std::string host_ = "192.168.88.3";
  int port_ = 8080;
  std::shared_ptr<EventLoop> loop_;
  std::unique_ptr<Acceptor> acceptor_;

  std::unordered_map<int, std::shared_ptr<Connection>> connections_;
  std::function<void(Connection *)> handle_onconnect_function_;
};
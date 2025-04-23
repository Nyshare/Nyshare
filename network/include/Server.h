#pragma once

#include <iostream>
#include <unordered_map>

#include "Acceptor.h"
#include "Connection.h"
#include "EventLoop.h"
#include "ThreadPool.h"

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
  std::string host_;
  int port_ = 8080;
  int thread_count_;
  std::shared_ptr<EventLoop> main_reactor_;
  std::vector<std::shared_ptr<EventLoop>> sub_reactors_;
  std::unique_ptr<Acceptor> acceptor_;
  std::unique_ptr<ThreadPool> thread_pool_;

  std::unordered_map<int, std::shared_ptr<Connection>> connections_;
  std::function<void(Connection *)> handle_onconnect_function_;
};
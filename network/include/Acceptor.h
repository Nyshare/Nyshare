#pragma once

#include <iostream>

#include "Channel.h"
#include "Socket.h"

class Acceptor {
 public:
  Acceptor(const std::shared_ptr<EventLoop> &loop);
  Acceptor(const Acceptor &obj) = delete;
  Acceptor &operator=(const Acceptor &obj) = delete;

  void bind(const std::string &host, int port);

  void bind(int port);

  void listen(int count = SOMAXCONN);

  void accept();

  void set_handle_new_connection_function(
      const std::function<void(int, const std::string &, int)> &function);

 private:
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<InetAddress> address_;
  std::unique_ptr<Channel> channel_;
  std::function<void(int, const std::string &, int)>
      handle_new_connection_function_;
};
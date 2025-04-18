#pragma once

#include <iostream>

#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"

class Connection {
 public:
  enum { CONNECT, DISCONNECT };
  explicit Connection(int fd, const std::string &host, int port,
                      const std::shared_ptr<EventLoop> &loop);

  void read();
  void write(const std::string &data);
  void write();

  void handle_onconnect();

  const std::string &read_buffer() const;

  int state();

  void set_write_buffer(const std::string &data);

  void set_handle_onconnect_function(
      const std::function<void(Connection *)> &function);

  void set_handle_disconnect_function(const std::function<void(int)> &function);

 private:
  static const int MAX_DATA_SIZE = 1024;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<InetAddress> address_;
  std::unique_ptr<Channel> channel_;
  int state_;
  std::string read_buffer_;
  std::string write_buffer_;
  std::function<void(Connection *)> handle_onconnect_function_;
  std::function<void(int)> handle_disconnect_function_;
};
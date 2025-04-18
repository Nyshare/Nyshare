#pragma once

#include <memory>

#include "InetAddress.h"

class Socket {
 public:
  Socket();
  Socket(Socket &&other) noexcept;
  explicit Socket(int fd);
  ~Socket();

  Socket(const Socket &obj) = delete;
  Socket &operator=(const Socket &obj) = delete;

  // 设置地址和端口复用选项
  // 开启地址复用: 允许多个套接字绑定到相同的地址和端口
  // 开启端口复用: 允许多个套接字绑定到相同的端口
  void reuse();

  // 用于将套接字设置为非阻塞模式。
  // 在非阻塞模式下，套接字操作（如读取或写入）不会阻塞程序执行，而是立即返回。
  void non_block();

  int fd() const;

 private:
  int fd_;
  bool reuse_addr_;
  bool reuse_port_;
};
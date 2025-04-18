#include "Socket.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Logger.h"

Socket::Socket() : fd_(-1), reuse_addr_(false), reuse_port_(false) {
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ <= 0) {
    error("[Socket]Failed create socket: %s", strerror(errno));
  }
}

Socket::Socket(Socket&& other) noexcept
    : fd_(-1), reuse_addr_(false), reuse_port_(false) {
  fd_ = other.fd_;
  reuse_addr_ = other.reuse_addr_;
  reuse_port_ = other.reuse_port_;

  other.fd_ = -1;
  other.reuse_addr_ = false;
  other.reuse_port_ = false;
}

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket() {
  if (fd_ != -1) {
    ::close(fd_);
    fd_ = -1;
  }
}

void Socket::reuse() {
  int opt = 1;
  // 开启地址复用: 允许多个套接字绑定到相同的地址和端口
  if (!reuse_addr_) {
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
      warn("[Socket] Failed to set SO_REUSEADDR on socket %d: %s", fd_,
           strerror(errno));
    } else {
      info("[Socket] Successfully set SO_REUSEADDR on socket %d", fd_);
    }
    reuse_addr_ = true;
  }

  // 开启端口复用
  if (!reuse_port_) {
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
      warn("[Socket] Failed to set SO_REUSEPORT on socket %d: %s", fd_,
           strerror(errno));
    } else {
      info("[Socket] Successfully set SO_REUSEPORT on socket %d", fd_);
    }
    reuse_port_ = true;
  }
}

void Socket::non_block() {
  // 获取当前文件描述符的状态标志
  int flags = fcntl(fd_, F_GETFL, 0);
  if (flags == -1) {
    warn("[Socket] Failed to get flags for socket %d: %s", fd_,
         strerror(errno));
    return;
  }

  // 检查当前是否已经设置为非阻塞模式
  if (flags & O_NONBLOCK) {
    info("[Socket] Socket %d is already in non-blocking mode.", fd_);
    return;  // 如果已经是非阻塞模式，直接返回
  }

  // 设置文件描述符为非阻塞模式
  if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
    warn("[Socket] Failed to set non-blocking mode for socket %d: %s", fd_,
         strerror(errno));
  } else {
    info("[Socket] Successfully set non-blocking mode for socket %d", fd_);
  }
}

int Socket::fd() const { return fd_; }

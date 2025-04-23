#include "Acceptor.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "Logger.h"

Acceptor::Acceptor(const std::shared_ptr<EventLoop>& loop)
    : socket_(std::make_unique<Socket>()),
      address_(nullptr),
      channel_(std::make_unique<Channel>(socket_->fd(), loop)),
      handle_new_connection_function_() {
  socket_->reuse();
  socket_->non_block();
  channel_->enable_read();
  channel_->set_handle_read_event_function([this]() { this->accept(); });
}

void Acceptor::bind(const std::string& host, int port) {
  if (address_) {
    address_.reset();
  }
  try {
    address_ = std::make_unique<InetAddress>(host, port);
  } catch (const std::exception& e) {
    fatal("[Acceptor] Failed to initialize address for %s:%d: %s", host.c_str(),
          port, e.what());
    throw std::runtime_error("[Socket] Failed to initialize address");
  }
  if (::bind(socket_->fd(), address_->addr(), address_->len()) == -1) {
    fatal("[Acceptor] Failed to bind to address %s:%d: %s", host.c_str(), port,
          strerror(errno));
    throw std::runtime_error("[Socket] Failed to bind to address");
  } else {
    info("[Acceptor] Successfully bound to address %s:%d", host.c_str(), port);
  }
}

void Acceptor::listen(int count) {
  if (::listen(socket_->fd(), count) == -1) {
    fatal("[Acceptor] Failed to listen on socket %d: %s", socket_->fd(),
          strerror(errno));
    throw std::runtime_error("[Acceptor] Failed listen");
  }
}

void Acceptor::accept() {
  InetAddress client_addr;
  socklen_t client_addr_len = client_addr.len();
  int client_fd = ::accept(socket_->fd(), client_addr.addr(), &client_addr_len);
  if (client_fd == -1) {
    warn("[Socket] Failed to accept client on socket %d: %s", socket_->fd(),
         strerror(errno));
    return;
  }
  info("[Server] Client %d conencted from %s:%d", client_fd,
       client_addr.host().c_str(), client_addr.port());
  if (handle_new_connection_function_) {
    handle_new_connection_function_(client_fd, client_addr.host(),
                                    client_addr.port());
  } else {
    warn("[Acceptor] New connection callback not set. Ignoring client %d",
         client_fd);
    ::close(client_fd);  // 防止 fd 泄漏
  }
}

void Acceptor::set_handle_new_connection_function(
    const std::function<void(int, const std::string&, int)>& function) {
  handle_new_connection_function_ = function;
}

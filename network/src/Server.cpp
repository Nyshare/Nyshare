#include "Server.h"

#include <iostream>
#include <nlohmann/json.hpp>

#include "Filer.h"
#include "Logger.h"

using json = nlohmann::json;

Server& Server::instance() {
  static Server server;
  return server;
}

void Server::run() { loop_->run(); }

void Server::set_handle_onconnect_function(
    const std::function<void(Connection*)>& function) {
  handle_onconnect_function_ = function;
}

Server::Server()
    : loop_(std::make_shared<EventLoop>()),
      acceptor_(std::make_unique<Acceptor>(loop_)) {
  load_config();
  if (host_.empty()) {
    acceptor_->bind(port_);
  } else {
    acceptor_->bind(host_, port_);
  }
  acceptor_->listen();
  acceptor_->set_handle_new_connection_function(
      [this](int fd, const std::string& host, int port) {
        this->new_connection(fd, host, port);
      });
}

void Server::new_connection(int fd, const std::string& host, int port) {
  std::shared_ptr<Connection> connection =
      std::make_shared<Connection>(fd, host, port, loop_);
  connection->set_handle_onconnect_function(
      [this](Connection* conn) { this->handle_onconnect_function_(conn); });
  connection->set_handle_disconnect_function(
      [this](int fd) { this->handle_disconnect(fd); });
  connections_.insert({fd, connection});
}

void Server::handle_disconnect(int fd) {
  auto conn = connections_.find(fd);
  if (conn == connections_.end()) {
    return;
  }
  info("[Server] Client %d disconnected", fd);
  connections_.erase(fd);
}

void Server::load_config() {
  std::string config_string = Filer::readFile("../config/network.json");
  if (config_string.empty()) {
    fatal("[Server] Not found network config json file");
    return;
  }
  json config = json::parse(config_string);
  host_ = config.value("host", "");
  port_ = config.value("port", 8080);
}

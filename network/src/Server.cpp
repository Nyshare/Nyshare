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

void Server::run() { main_reactor_->run(); }

void Server::set_handle_onconnect_function(
    const std::function<void(Connection*)>& function) {
  handle_onconnect_function_ = function;
}

Server::Server()
    : host_("0.0.0.0"),
      port_(8080),
      thread_count_(std::thread::hardware_concurrency()),
      main_reactor_(std::make_shared<EventLoop>()),
      acceptor_(std::make_unique<Acceptor>(main_reactor_)) {
  load_config();
  acceptor_->bind(host_, port_);
  acceptor_->listen();
  acceptor_->set_handle_new_connection_function(
      [this](int fd, const std::string& host, int port) {
        this->new_connection(fd, host, port);
      });

  thread_pool_ =
      std::make_unique<ThreadPool>(std::thread::hardware_concurrency());
  for (int i = 0; i < thread_count_; ++i) {
    auto sub_reactor = std::make_shared<EventLoop>();
    sub_reactors_.emplace_back(sub_reactor);
    thread_pool_->enqueue([sub_reactor]() { sub_reactor->run(); });
  }
}

void Server::new_connection(int fd, const std::string& host, int port) {
  std::shared_ptr<Connection> connection = std::make_shared<Connection>(
      fd, host, port, sub_reactors_[fd % thread_count_]);
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
  host_ = config.value("host", "0.0.0.0");
  port_ = config.value("port", 8080);
  thread_count_ =
      config.value("thread_count", std::thread::hardware_concurrency());
}

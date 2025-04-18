#pragma once
#include <netinet/in.h>

#include <iostream>
#include <string>

class InetAddress {
 public:
  InetAddress();
  InetAddress(const std::string &host, int port);
  InetAddress(int port);

  sockaddr *addr() const;
  socklen_t len() const;

  std::string host() const;

  int port() const;

 private:
  struct sockaddr_in addr_;
};
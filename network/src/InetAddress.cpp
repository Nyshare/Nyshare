#include "InetAddress.h"

#include <arpa/inet.h>
#include <string.h>

InetAddress::InetAddress() { memset(&addr_, 0, sizeof(addr_)); }

InetAddress::InetAddress(const std::string& host, int port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = inet_addr(host.c_str());
  addr_.sin_port = htons(port);
}

InetAddress::InetAddress(int port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = INADDR_ANY;
  addr_.sin_port = htons(port);
}

sockaddr* InetAddress::addr() const { return (sockaddr*)&addr_; }

socklen_t InetAddress::len() const { return sizeof(addr_); }

std::string InetAddress::host() const { return inet_ntoa(addr_.sin_addr); }

int InetAddress::port() const { return ntohs(addr_.sin_port); }

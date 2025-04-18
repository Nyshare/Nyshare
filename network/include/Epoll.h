#pragma once

#include <sys/epoll.h>

#include <iostream>
#include <vector>

#include "Channel.h"
#include "Socket.h"

class Channel;
class Epoll {
 public:
  Epoll();
  Epoll(const Epoll &) = delete;
  Epoll &operator=(const Epoll &) = delete;
  ~Epoll();

  void update_channel(Channel *ch);

  void del(int fd);

  std::vector<Channel *> wait(int timeout = -1);

 private:
  static const int MAX_EVENTS_SIZE = 1024;
  int epfd_;
  epoll_event events_[MAX_EVENTS_SIZE];
};
#pragma once

#include <memory>

#include "Epoll.h"

class Channel;
class Epoll;
class EventLoop {
 public:
  EventLoop();
  EventLoop(const EventLoop&) = delete;
  EventLoop& operator=(const EventLoop&) = delete;

  void run();

  void updata_channel(Channel* ch);

  void delete_channel(Channel* ch);

 private:
  std::unique_ptr<Epoll> epoll_;
};
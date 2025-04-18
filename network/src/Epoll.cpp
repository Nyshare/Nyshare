#include "Epoll.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "Logger.h"

Epoll::Epoll() : epfd_(-1) {
  epfd_ = epoll_create(EPOLL_CLOEXEC);
  memset(events_, 0, sizeof(events_));
  if (epfd_ == -1) {
    fatal("[Epoll] Failed to create epoll instance: %s", strerror(errno));
    throw std::runtime_error("Failed to create epoll instance");
  }
}

Epoll::~Epoll() {
  if (epfd_ != -1) {
    close(epfd_);
    epfd_ = -1;
  }
}

void Epoll::update_channel(Channel* ch) {
  epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.data.ptr = ch;
  ev.events = ch->events();

  int fd = ch->fd();

  if (ch->in_epoll()) {
    if (epoll_ctl(epfd_, EPOLL_CTL_MOD, ch->fd(), &ev) == -1) {
      warn("[Epoll] Failed to EPOLL_CTL_MOD fd %d: %s", fd, strerror(errno));
    }
  } else {
    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1) {
      warn("[Epoll] Failed to EPOLL_CTL_ADD fd %d: %s", fd, strerror(errno));
    }
    ch->set_in_epoll();
  }
}

void Epoll::del(int fd) {
  if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    warn("[Epoll] Failed to remove socket %d from epoll: %s", fd,
         strerror(errno));
  }
}

std::vector<Channel*> Epoll::wait(int timeout) {
  std::vector<Channel*> reevents;
  memset(events_, 0, sizeof(events_));
  int count = epoll_wait(epfd_, events_, MAX_EVENTS_SIZE, timeout);
  if (count == -1) {
    error("[Epoll] epoll_wait failed: %s", strerror(errno));
    return reevents;
  }

  reevents.reserve(count);
  for (int i = 0; i < count; ++i) {
    Channel* ch = (Channel*)events_[i].data.ptr;
    ch->set_reevents(events_[i].events);
    reevents.emplace_back(ch);
  }
  return reevents;
}

#include "Channel.h"

#include "Logger.h"

Channel::Channel(int fd, const std::shared_ptr<EventLoop> &loop)
    : fd_(fd), loop_(loop), in_epoll_(false), events_(0), reevents_(0) {}

void Channel::enable_read() {
  if (events_ & EPOLLIN) {
    return;
  }
  events_ |= EPOLLIN;
  loop_->updata_channel(this);
}

void Channel::enable_write() {
  if (events_ & EPOLLOUT) {
    return;
  }
  events_ |= EPOLLOUT;
  loop_->updata_channel(this);
}

void Channel::disable_write() {
  if (events_ & EPOLLOUT) {
    events_ &= ~EPOLLOUT;
    loop_->updata_channel(this);
  }
}

void Channel::handle_read_event() {
  if (handle_read_event_function_) {
    handle_read_event_function_();
  } else {
    warn("[Channel] Handle function is null");
  }
}

void Channel::handle_write_event() {
  if (handle_write_event_function_) {
    handle_read_event_function_();
  } else {
    warn("[Channel] Handle write function is null");
  }
}

void Channel::handle_disconnect() {
  if (handle_disconenct_function_) {
    loop_->delete_channel(this);
    handle_disconenct_function_(fd_);
  } else {
    error("[Channel] Handle disconnection function is null");
  }
}

void Channel::useET() {
  if (events_ & EPOLLET) {
    return;
  }
  events_ |= EPOLLET;
  loop_->updata_channel(this);
}

int Channel::fd() const { return fd_; }

uint32_t Channel::events() const { return events_; }

uint32_t Channel::reevents() const { return reevents_; }

bool Channel::in_epoll() const { return in_epoll_; }

void Channel::set_in_epoll() { in_epoll_ = true; }

void Channel::set_reevents(uint32_t reevents) { reevents_ = reevents; }

void Channel::set_handle_read_event_function(
    const std::function<void()> &function) {
  handle_read_event_function_ = function;
}

void Channel::set_handle_write_event_function(
    const std::function<void()> &function) {
  handle_write_event_function_ = function;
}

void Channel::set_handle_disconnect_function(
    const std::function<void(int)> &function) {
  handle_disconenct_function_ = function;
}

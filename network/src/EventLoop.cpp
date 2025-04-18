#include "EventLoop.h"

#include "Logger.h"

EventLoop::EventLoop() : epoll_(std::make_unique<Epoll>()) {}

void EventLoop::run() {
  while (true) {
    std::vector<Channel*> channels = epoll_->wait(10);
    for (auto& ch : channels) {
      if (ch->reevents() & EPOLLIN) {
        // 处理读事件
        ch->handle_read_event();
      } else if (ch->reevents() & EPOLLOUT) {
        // 处理写事件
        ch->handle_write_event();
      } else if (ch->reevents() & EPOLLHUP) {
        // 客户端断开连接
        ch->handle_disconnect();
        info("[EventLoop] Client %d disconnected (HUP)", ch->fd());
      } else if (ch->reevents() & EPOLLERR) {
        // 发生错误，断开连接
        ch->handle_disconnect();
        error("[EventLoop] Socket error on %d, so disconnected", ch->fd());
      } else {
        // 未知错误，断开连接
        ch->handle_disconnect();
        error("[EventLoop] Unknown event on %d, so disconnected", ch->fd());
      }
    }
  }
}

void EventLoop::updata_channel(Channel* ch) { epoll_->update_channel(ch); }

void EventLoop::delete_channel(Channel* ch) { epoll_->del(ch->fd()); }

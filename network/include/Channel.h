#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include "Epoll.h"
#include "EventLoop.h"

class Epoll;
class EventLoop;
class Channel {
 public:
  explicit Channel(int fd, const std::shared_ptr<EventLoop> &loop);

  // 开启监听读事件
  void enable_read();

  // 开启监听写事件
  void enable_write();

  // 关闭监听写事件
  void disable_write();

  void handle_read_event();

  void handle_write_event();

  void handle_disconnect();

  // 启用水平触发
  void useET();

  int fd() const;

  uint32_t events() const;

  uint32_t reevents() const;

  bool in_epoll() const;

  void set_in_epoll();

  void set_reevents(uint32_t reevents);

  void set_handle_read_event_function(const std::function<void()> &function);
  void set_handle_write_event_function(const std::function<void()> &function);
  void set_handle_disconnect_function(const std::function<void(int)> &function);

 private:
  int fd_;                           // 管理的fd，只有管理权没有拥有权
  std::shared_ptr<EventLoop> loop_;  // 所属事件循环
  bool in_epoll_;                    // 在红黑树上的标志
  uint32_t events_;                  // 监听的事件
  uint32_t reevents_;                // 返回的事件

  std::function<void()> handle_read_event_function_;     // 处理读事件
  std::function<void()> handle_write_event_function_;    // 处理写事件
  std::function<void(int)> handle_disconenct_function_;  // 处理连接断开
};
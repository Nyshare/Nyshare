#include "Connection.h"

#include <string.h>
#include <unistd.h>

#include "Logger.h"

Connection::Connection(int fd, const std::string& host, int port,
                       const std::shared_ptr<EventLoop>& loop)
    : socket_(std::make_unique<Socket>(fd)),
      address_(std::make_unique<InetAddress>(host, port)),
      channel_(std::make_unique<Channel>(fd, loop)),
      state_(CONNECT),
      read_buffer_(),
      write_buffer_(),
      handle_onconnect_function_(),
      handle_disconnect_function_() {
  socket_->non_block();
  channel_->enable_read();
  channel_->useET();
  channel_->set_handle_read_event_function(
      [this]() { this->handle_onconnect_function_(this); });
  channel_->set_handle_write_event_function([this]() { this->write(); });
  channel_->set_handle_disconnect_function(
      [this](int fd) { this->handle_disconnect_function_(fd); });
}

void Connection::read() {
  read_buffer_.clear();  // 清空当前缓冲区
  char data[MAX_DATA_SIZE];

  while (true) {
    memset(data, 0, sizeof(data));  // 清空本次读取缓冲区
    int read_bytes = ::read(socket_->fd(), data, sizeof(data));

    if (read_bytes > 0) {
      // 读取到数据，追加到缓冲区
      read_buffer_.append(data, read_bytes);
    } else if (read_bytes == 0) {
      // 客户端断开连接
      state_ = DISCONNECT;
      channel_->handle_disconnect();
      break;
    } else if (read_bytes == -1 && errno == EINTR) {
      // 读取被信号中断，继续读取
      continue;
    } else if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // 非阻塞模式下没有数据可读，退出循环
      break;
    } else if (read_bytes == -1) {
      // 其他错误，打印具体错误信息
      warn("[Connection] Socket read error: %s", strerror(errno));
      state_ = DISCONNECT;
      channel_->handle_disconnect();
      break;
    } else {
      // 理论上不会到这里，防止遗漏情况
      warn("[Connection] Unexpected socket read behavior");
      state_ = DISCONNECT;
      channel_->handle_disconnect();
      break;
    }
  }
}

void Connection::write(const std::string& data) {
  write_buffer_ = data;
  write();
}

void Connection::write() {
  // 获取要发送数据的长度
  int data_bytes = write_buffer_.size();
  if (data_bytes == 0) {
    return;
  }

  // 准备数据缓冲区
  const char* data = write_buffer_.data();

  int left_bytes = data_bytes;  // 剩余需要写入的字节数
  int write_bytes = 0;
  while (left_bytes > 0) {
    int written = data_bytes - left_bytes;
    write_bytes = ::write(socket_->fd(), data + written, left_bytes);
    if (write_bytes > 0) {
      left_bytes -= write_bytes;
    } else if (write_bytes == 0) {
      // 客户端断开连接，暂不处理
      info("[Socket] Client %s:%d disconnected", address_->host(),
           address_->port());
      state_ = DISCONNECT;
      break;
    } else if (write_bytes == -1 && errno == EINTR) {
      continue;  // 被信号打断，继续尝试写
    } else if (write_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // 非阻塞写失败，等待下一次可写事件
      write_buffer_.erase(0, data_bytes - left_bytes);
      channel_->enable_write();
      break;
    } else {
      warn("[Socket] Write error:%s", strerror(errno));
      state_ = DISCONNECT;
      channel_->handle_disconnect();
      break;
    }
  }
  if (left_bytes == 0) {
    write_buffer_.clear();
    channel_->disable_write();  // 全部数据发送完毕，关闭监听可写事件
  }
}

void Connection::handle_onconnect() { handle_onconnect_function_(this); }

const std::string& Connection::read_buffer() const { return read_buffer_; }

int Connection::state() { return state_; }

void Connection::set_write_buffer(const std::string& data) {
  write_buffer_ = data;
}

void Connection::set_handle_onconnect_function(
    const std::function<void(Connection*)>& function) {
  handle_onconnect_function_ = function;
}

void Connection::set_handle_disconnect_function(
    const std::function<void(int)>& function) {
  handle_disconnect_function_ = function;
}

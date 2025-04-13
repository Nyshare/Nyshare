#include "Logger.h"

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <iostream>

const char *Logger::s_level[LEVEL_COUNT]{"DEBUG", "INFO", "WARN", "ERROR",
                                         "FATAL"};

void Logger::log(Level level, const char *file, int line,
                 const std::string &content) {
  if (m_fout.fail()) {
    throw std::logic_error("open file failed " + m_filename);
  }
  if (level < m_level) {
    return;
  }
  if (m_len >= m_max && m_max > 0) {
    rotate();
    m_len = 0;
  }

  // 获取格式化时间戳
  time_t ticks = time(NULL);
  struct tm *ptm = localtime(&ticks);
  char timestamp[32];
  memset(timestamp, 0, sizeof(timestamp));
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ptm);

  // 处理时间 级别 文件：行号
  const char *ftm = "[%s] [%s] %s:%d - ";
  // 计算所需大小
  int size = snprintf(NULL, 0, ftm, timestamp, s_level[level], file, line);
  if (size > 0) {
    // 构造缓冲区写入日志
    char *buffer = new char[size + 1];
    memset(buffer, 0, size + 1);
    snprintf(buffer, size + 1, ftm, timestamp, s_level[level], file, line);
    m_fout << buffer;
    m_len += size;
    delete[] buffer;
  }

  // 处理内容
  m_len += content.size();
  m_fout << content.c_str() << std::endl;
  m_fout.flush();
}

Logger &Logger::getInstance() {
  static Logger logger;
  return logger;
}

void Logger::open(const std::string &filename) {
  m_filename = filename;
  m_fout.open(m_filename, std::ios::app);
  if (m_fout.fail()) {
    throw std::logic_error("open file failed:" + filename);
  }
}

void Logger::close() { m_fout.close(); }

void Logger::set_level(Level level) { m_level = level; }

void Logger::set_max(int max_len) { m_max = max_len; }

void Logger::add_log(Level level, const char *file, int line,
                     const char *format, ...) {
  va_list arg_ptr;
  va_start(arg_ptr, format);
  int size = vsnprintf(NULL, 0, format, arg_ptr);
  va_end(arg_ptr);
  if (size < 0) {
    return;
  }
  std::string content(size + 1, '\0');
  va_start(arg_ptr, format);
  vsnprintf(&content[0], content.size(), format, arg_ptr);
  va_end(arg_ptr);
  auto task = [this, level, file = std::string(file), line,
               logMessage = std::move(content)]() {
    this->log(level, file.c_str(), line, logMessage);
  };
  {
    std::unique_lock<std::mutex> locker(m_queueMutex);
    m_taskQueue.push(std::move(task));
  }
  m_queueCond.notify_one();
}

Logger::Logger()
    : m_level(DEBUG), m_max(10 * 1024 * 1024), m_len(0), m_stop(false) {
  m_filename = "./server.log";
  open(m_filename);
  m_workThread = std::thread(&Logger::output_log, this);
  m_workThread.detach();
}

Logger::~Logger() {
  close();
  {
    std::unique_lock<std::mutex> locker(m_queueMutex);
    m_stop = true;
  }
  m_queueCond.notify_all();
  if (m_workThread.joinable()) {
    m_workThread.join();
  }
}

void Logger::rotate() {
  close();
  time_t ticks = time(NULL);
  struct tm *ptm = localtime(&ticks);
  char timestamp[32];
  memset(timestamp, 0, sizeof(timestamp));
  strftime(timestamp, sizeof(timestamp), ".%Y-%m-%d_%H-%M-%S", ptm);
  std::string filename = m_filename + timestamp;
  if (rename(m_filename.c_str(), filename.c_str()) != 0) {
    throw std::logic_error("rename log file failed:" +
                           std::string(strerror(errno)));
  }
  open(m_filename);
}

void Logger::output_log() {
  while (true) {
    std::unique_lock<std::mutex> locker(m_queueMutex);
    m_queueCond.wait(locker,
                     [this]() { return !m_taskQueue.empty() || m_stop; });
    if (m_stop && m_taskQueue.empty()) {
      return;
    }
    auto task = std::move(m_taskQueue.front());
    m_taskQueue.pop();
    locker.unlock();
    // std::cout << "out put log\n";
    task();
  }
}

#pragma once
#include <condition_variable>
#include <fstream>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#define debug(format, ...)                                                 \
  Logger::getInstance().add_log(Logger::DEBUG, __FILE__, __LINE__, format, \
                                ##__VA_ARGS__);
#define info(format, ...)                                                 \
  Logger::getInstance().add_log(Logger::INFO, __FILE__, __LINE__, format, \
                                ##__VA_ARGS__);
#define warn(format, ...)                                                 \
  Logger::getInstance().add_log(Logger::WARN, __FILE__, __LINE__, format, \
                                ##__VA_ARGS__);
#define error(format, ...)                                                 \
  Logger::getInstance().add_log(Logger::ERROR, __FILE__, __LINE__, format, \
                                ##__VA_ARGS__);
#define fatal(format, ...)                                                 \
  Logger::getInstance().add_log(Logger::FATAL, __FILE__, __LINE__, format, \
                                ##__VA_ARGS__);

class Logger {
 public:
  enum Level { DEBUG = 0, INFO, WARN, ERROR, FATAL, LEVEL_COUNT };
  // 记录日志
  void log(Level level, const char *file, int line, const std::string &content);
  // 单例模式，不允许拷贝
  Logger(const Logger &obj) = delete;
  const Logger &operator=(const Logger &obj) = delete;
  // 获取单例对象
  static Logger &getInstance();
  // 打开日志文件
  void open(const std::string &filename);
  // 关闭日志文件
  void close();
  // 设置日志级别
  void set_level(Level level);
  void set_max(int max_len);
  void add_log(Level level, const char *file, int line, const char *format,
               ...);

 private:
  Logger();
  ~Logger();
  // 日志翻滚
  void rotate();
  void output_log();

 private:
  std::queue<std::function<void()>> m_taskQueue;
  std::mutex m_queueMutex;
  std::condition_variable m_queueCond;
  std::thread m_workThread;
  bool m_stop;
  int m_max;
  int m_len;
  Level m_level;                            // 设置日志的级别，默认DEBUG
  std::string m_filename;                   // 日志文件名
  std::ofstream m_fout;                     // 写入日志文件句柄
  static const char *s_level[LEVEL_COUNT];  // 保存日志级别的字符串
};
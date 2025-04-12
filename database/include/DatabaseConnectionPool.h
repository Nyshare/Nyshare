#pragma once

#include <condition_variable>
#include <memory>
#include <queue>
#include <thread>

#include "DatabaseConnection.h"

class DatabaseConnectionPool {
 public:
  ~DatabaseConnectionPool();
  // 获取单例对象
  static DatabaseConnectionPool& getInstance();

  // 获取连接（线程安全，自定义删除器自动回收）
  std::shared_ptr<DatabaseConnection> acquire();

 private:
  DatabaseConnectionPool();

  // 删除拷贝构造
  DatabaseConnectionPool(const DatabaseConnection& object) = delete;

  // 删除赋值构造
  DatabaseConnectionPool& operator=(const DatabaseConnectionPool& object) =
      delete;

  // 加载数据库配置
  void load_config();

  // 生产连接线程工作函数
  void produce();

  // 回收连接线程工作函数
  void recycle();

  // 添加一个数据库连接
  bool create_and_enqueue_connection();

 private:
  std::string m_host;                            // 数据库服务器IP
  std::string m_user;                            // 用户名
  std::string m_passwd;                          // 密码
  std::string m_db;                              // 数据库库名
  unsigned int m_port;                           // 端口
  int m_min_size;                                // 最小连接数
  int m_max_size;                                // 最大连接数
  int m_connection_count;                        // 当前连接数量
  int m_max_idle_time;                           // 最大连接空闲时间
  int m_timeout;                                 // 获取连接的线程等待超时时间
  bool m_running;                                // 连接池工作标志
  std::thread m_producer;                        // 生产连接线程
  std::thread m_recycler;                        // 回收连接线程
  std::mutex m_queue_lock;                       // 连接队列锁
  std::condition_variable m_condition_variable;  // 线程阻塞条件变量
  std::queue<std::shared_ptr<DatabaseConnection>>
      m_connections_queue;  // 连接队列
};
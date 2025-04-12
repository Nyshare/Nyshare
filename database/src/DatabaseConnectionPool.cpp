#include "DatabaseConnectionPool.h"

#include "Filer.h"
#include "Logger.h"

DatabaseConnectionPool::~DatabaseConnectionPool() {
  m_running = false;
  m_condition_variable.notify_all();
  if (m_producer.joinable()) {
    m_producer.join();
  }
  if (m_recycler.joinable()) {
    m_recycler.join();
  }
  std::lock_guard<std::mutex> locker(m_queue_lock);
  while (!m_connections_queue.empty()) {
    auto connection = m_connections_queue.front();
    m_connections_queue.pop();
    connection.reset();
    m_connection_count--;
  }
}

DatabaseConnectionPool& DatabaseConnectionPool::getInstance() {
  static DatabaseConnectionPool databaseConnectionPool;
  return databaseConnectionPool;
}

std::shared_ptr<DatabaseConnection> DatabaseConnectionPool::acquire() {
  std::unique_lock<std::mutex> locker(m_queue_lock);
  while (m_connections_queue.empty()) {
    if (m_condition_variable.wait_for(locker, milliseconds(m_timeout)) ==
            std::cv_status::timeout &&
        m_connections_queue.empty()) {
      continue;
    }
  }

  auto origin_connection = m_connections_queue.front();
  m_connections_queue.pop();
  m_condition_variable.notify_all();

  auto deleter = [this, connection = origin_connection](DatabaseConnection*) {
    std::unique_lock<std::mutex> locker(m_queue_lock);
    connection->refresh();
    m_connections_queue.push(connection);
  };
  std::shared_ptr<DatabaseConnection> connection(origin_connection.get(),
                                                 deleter);
  return connection;
}

DatabaseConnectionPool::DatabaseConnectionPool()
    : m_port(3306),
      m_min_size(std::thread::hardware_concurrency()),
      m_max_size(std::thread::hardware_concurrency()),
      m_max_idle_time(2 * 60 * 1000),
      m_timeout(1000),
      m_running(true) {
  load_config();
  for (int i = 0; i < m_min_size; ++i) {
    create_and_enqueue_connection();
  }
  info("[DatabaseConnectionPool] Initialized with %d connections", m_min_size);
  m_producer = std::thread(&DatabaseConnectionPool::produce, this);
  m_recycler = std::thread(&DatabaseConnectionPool::recycle, this);
  m_producer.detach();
  m_recycler.detach();
}

void DatabaseConnectionPool::load_config() {
  std::string config = Filer::readFile("./database.json");
  if (config.empty()) {
    fatal(
        "[DatabaseConnectionPool] Database config file is nonexist or empty:%s",
        Filer::getAbsolutePath("./database.json"));
    throw std::logic_error(
        "[DatabaseConnectionPool] Database config file is nonexist or empty");
  }
  Json configJson;
  configJson.parse(config);
  m_host = configJson["host"].asString();
  m_user = configJson["user"].asString();
  m_passwd = configJson["passwd"].asString();
  m_db = configJson["db"].asString();
  m_port = configJson["port"].asInt();
  m_min_size = configJson["min_size"].asInt();
  m_max_size = configJson["max_size"].asInt();
  m_max_idle_time = configJson["idle_time"].asInt();
  m_timeout = configJson["timeout"].asInt();
  // 检查必要配置，其他配置可设置默认值
  if (m_host.empty() || m_user.empty() || m_passwd.empty() || m_db.empty()) {
    fatal("[DatabaseConnectionPool] Database config file is invalid");
    throw std::logic_error(
        "[DatabaseConnectionPool] Database config file is invalid");
  }
}

void DatabaseConnectionPool::produce() {
  while (m_running) {
    std::unique_lock<std::mutex> locker(m_queue_lock);
    while (m_connections_queue.size() >= m_min_size && m_running) {
      m_condition_variable.wait(locker);
    }
    if (create_and_enqueue_connection()) {
      m_condition_variable.notify_all();
    }
  }
}

void DatabaseConnectionPool::recycle() {
  while (m_running) {
    std::unique_lock<std::mutex> locker(m_queue_lock);
    // 如果连接数小于最小连接数，则回收线程阻塞，无需进行回收工作
    while (m_connections_queue.size() <= m_min_size && m_running) {
      m_condition_variable.wait(locker);
    }
    // 从队头取出连接
    auto connection = m_connections_queue.front();
    long long idle_time = connection->idle_time();
    if (idle_time >= m_max_idle_time) {
      // 连接空闲时间大于最大空闲时间，回收连接
      m_connections_queue.pop();
      connection.reset();
      m_connection_count--;
    } else {
      // 回收线程阻塞时间 = 最大空闲时间 -连接空闲时间
      // 线程阻塞的时间即是队列中最快达到最大空闲时间所需的时间
      locker.unlock();
      std::this_thread::sleep_for(milliseconds(m_max_idle_time - idle_time));
    }
  }
}

bool DatabaseConnectionPool::create_and_enqueue_connection() {
  if (m_connection_count >= m_max_size) {
    // 已连接数量大于最大连接数，不再创建新连接
    return false;
  }
  std::shared_ptr<DatabaseConnection> connection =
      std::make_shared<DatabaseConnection>();
  if (!connection->connect(m_host, m_user, m_passwd, m_db, m_port)) {
    return false;
  }
  connection->refresh();
  m_connections_queue.push(std::move(connection));
  m_connection_count++;
  return true;
}

#pragma once

#include <mysql/mysql.h>
#include <string.h>

#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>

#include "Logger.h"

using namespace std::chrono;
using json = nlohmann::json;

class DatabaseConnection {
 public:
  DatabaseConnection();
  DatabaseConnection(const DatabaseConnection &object) = delete;
  DatabaseConnection &operator=(const DatabaseConnection &object) = delete;

  // 连接数据库
  bool connect(const std::string &host, const std::string &user,
               const std::string &passwd, const std::string &db,
               unsigned int port = 3306);

  // 开启事务
  bool transaction();

  // 执行SQL语句
  template <class... Args>
  bool query(const std::string &sql, Args &&...args);

  // 提交事务
  bool commit();

  // 事务回滚
  bool rollback();

  // 判断结果集是否为空
  bool empty();

  // 获取JSON格式的结果集
  std::vector<json> result();

  // 刷新连接空闲起始时间点
  void refresh();

  // 计算空闲时间
  long long idle_time();

 private:
  // 准备预处理语句
  bool prepare(const std::string &sql);

  // 绑定参数
  template <typename... Args>
  bool bind(Args &&...args);

  // 执行预处理语句
  bool execute();

  // 辅助函数递归处理每个参数
  bool bind_internal(size_t index);
  template <typename T, typename... Rest>
  bool bind_internal(size_t index, T &&value, Rest &&...rest);

  void mysql_deleter(MYSQL *conn);
  void mysql_res_deleter(MYSQL_RES *result);
  void mysql_stmt_deleter(MYSQL_STMT *stmt);

 private:
  std::unique_ptr<MYSQL, std::function<void(MYSQL *)>> m_conn;  // 底层连接指针
  std::unique_ptr<MYSQL_RES, std::function<void(MYSQL_RES *)>>
      m_result;                          // 结果集指针
  steady_clock::time_point m_idle_time;  // 连接空闲起始时间点
  std::unique_ptr<MYSQL_STMT, std::function<void(MYSQL_STMT *)>>
      m_stmt;                             // 预处理句柄
  std::vector<MYSQL_BIND> m_bind_params;  // 参数绑定信息
  std::vector<std::shared_ptr<void>>
      m_bind_buffers;  // 实际参数值的 buffer，避免悬空引用
};

template <class... Args>
inline bool DatabaseConnection::query(const std::string &sql, Args &&...args) {
  if (!prepare(sql)) {
    return false;
  }
  if (!bind(std::forward<Args>(args)...)) {
    return false;
  }
  if (!execute()) {
    return false;
  }
  return true;
}

template <typename... Args>
inline bool DatabaseConnection::bind(Args &&...args) {
  // 获取实际参数数量
  constexpr size_t param_count = sizeof...(Args);

  // 验证参数数量是否与 SQL 中占位符 ? 数量一致
  if (mysql_stmt_param_count(m_stmt.get()) != param_count) {
    warn(
        "[DatabaseConnection] Mismatch in parameter count: expected %llu, got "
        "%zu",
        mysql_stmt_param_count(m_stmt.get()), param_count);
    return false;
  }

  // 清理旧的参数绑定和缓存数据，避免悬空引用
  m_bind_params.clear();   // MYSQL_BIND 数组
  m_bind_buffers.clear();  // 每个参数对应的实际内存空间

  // 依次处理每个参数并构造 MYSQL_BIND，保存到 m_bind_params
  bool success = bind_internal(0, std::forward<Args>(args)...);
  if (!success) {
    return false;
  }

  // 将所有绑定参数应用到预处理语句中
  if (mysql_stmt_bind_param(m_stmt.get(), m_bind_params.data())) {
    warn("[DatabaseConnection] Failed to bind parameters: %s",
         mysql_stmt_error(m_stmt.get()));
    return false;
  }

  debug("[DatabaseConnection] Bind params is successful!");
  return true;
}

template <typename T, typename... Rest>
inline bool DatabaseConnection::bind_internal(size_t index, T &&value,
                                              Rest &&...rest) {
  MYSQL_BIND bind;
  memset(&bind, 0, sizeof(bind));  // 初始化绑定结构体

  // 创建一个新的共享 buffer 保存当前参数值，防止悬空引用
  m_bind_buffers.emplace_back();
  auto &buffer = m_bind_buffers.back();

  // 类型判断并处理：目前支持 int, double, std::string
  if constexpr (std::is_same_v<std::decay_t<T>, int>) {
    // 拷贝值到共享 buffer，并设置 MYSQL 类型与 buffer 地址
    buffer = std::make_shared<int>(value);
    bind.buffer_type = MYSQL_TYPE_LONG;
    bind.buffer = buffer.get();
  } else if constexpr (std::is_same_v<std::decay_t<T>, double>) {
    buffer = std::make_shared<double>(value);
    bind.buffer_type = MYSQL_TYPE_DOUBLE;
    bind.buffer = buffer.get();
  } else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
    // 字符串特殊处理：需要 const_cast 并设置 buffer_length
    buffer = std::make_shared<std::string>(value);
    bind.buffer_type = MYSQL_TYPE_STRING;
    bind.buffer =
        const_cast<char *>(static_cast<std::string *>(buffer.get())->c_str());
    bind.buffer_length = static_cast<std::string *>(buffer.get())->length();
  } else {
    // 不支持的类型发出警告
    warn("[DatabaseConnection] Unsupported bind type at index %zu", index);
    return false;
  }

  // 将当前构造好的 bind 添加到绑定参数数组中
  m_bind_params.push_back(bind);

  // 递归处理下一个参数
  return bind_internal(index + 1, std::forward<Rest>(rest)...);
}

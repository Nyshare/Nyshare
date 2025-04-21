#include "DatabaseConnection.h"

#include <string.h>

#include <iostream>
#include <string>

#include "Logger.h"

DatabaseConnection::DatabaseConnection() {
  m_conn = std::unique_ptr<MYSQL, std::function<void(MYSQL*)>>(
      mysql_init(nullptr), [](MYSQL* conn) {
        if (conn != nullptr) {
          mysql_close(conn);
        }
      });

  m_result = std::unique_ptr<MYSQL_RES, std::function<void(MYSQL_RES*)>>(
      nullptr,  // 初始为空，之后查询时再赋值
      [](MYSQL_RES* result) {
        if (result != nullptr) {
          mysql_free_result(result);
        }
      });

  m_idle_time = steady_clock::now();

  m_stmt = std::unique_ptr<MYSQL_STMT, std::function<void(MYSQL_STMT*)>>(
      mysql_stmt_init(m_conn.get()), [](MYSQL_STMT* stmt) {
        if (stmt != nullptr) {
          (void)mysql_stmt_close(stmt);  // 忽略返回值
        }
      });
  if (m_conn == nullptr) {
    error("[DatabaseConnection] MySQL failed connect");
  }
  if (m_stmt == nullptr) {
    error("[DatabaseConnection] MySQL failed initialize stmt");
  }
}

bool DatabaseConnection::connect(const std::string& host,
                                 const std::string& user,
                                 const std::string& passwd,
                                 const std::string& db, unsigned int port) {
  if (mysql_real_connect(m_conn.get(), host.c_str(), user.c_str(),
                         passwd.c_str(), db.c_str(), port, nullptr,
                         0) == nullptr) {
    error("[DatabaseConnection] MySQL failed to connect to %s:%u", host.c_str(),
          port);
    return false;
  }
  return true;
}

bool DatabaseConnection::transaction() {
  return mysql_autocommit(m_conn.get(), false);
}

bool DatabaseConnection::commit() { return !mysql_commit(m_conn.get()); }

bool DatabaseConnection::rollback() { return !mysql_rollback(m_conn.get()); }

bool DatabaseConnection::empty() {
  if (m_stmt == nullptr || m_result == nullptr) {
    return true;
  }
  return mysql_stmt_num_rows(m_stmt.get()) == 0;
}

std::vector<json> DatabaseConnection::result() {
  std::vector<json> result;
  if (m_result == nullptr) {
    warn("[DatabaseConnection] Failed get result because result is nullptr");
    return result;
  }
  // 获取结果集的行数
  int row_count = mysql_stmt_num_rows(m_stmt.get());
  // 获取结果集的列数
  int column_count = mysql_num_fields(m_result.get());
  // 获取结果集字段结构体数组
  MYSQL_FIELD* fields = mysql_fetch_fields(m_result.get());
  // 字段值数组绑定信息
  std::vector<MYSQL_BIND> binds(column_count);
  std::vector<std::vector<char>> buffers(column_count);
  std::vector<char> is_null_flags(column_count);
  std::vector<unsigned long> lengths(column_count);

  for (int i = 0; i < column_count; ++i) {
    MYSQL_BIND& bind = binds[i];
    memset(&bind, 0, sizeof(bind));
    unsigned long len = fields[i].length;

    // 为不同类型的字段分配合理长度
    switch (fields[i].type) {
      case MYSQL_TYPE_TINY:
        len = sizeof(int8_t);
        break;
      case MYSQL_TYPE_SHORT:
        len = sizeof(int16_t);
        break;
      case MYSQL_TYPE_LONG:
        len = sizeof(int32_t);
        break;
      case MYSQL_TYPE_LONGLONG:
        len = sizeof(int64_t);
        break;
      case MYSQL_TYPE_FLOAT:
        len = sizeof(float);
        break;
      case MYSQL_TYPE_DOUBLE:
        len = sizeof(double);
        break;
      default:
        if (len < 128) len = 128;  // 防止过小，给个下限
        break;
    }
    buffers[i].resize(len);
    bind.buffer_type = fields[i].type;
    bind.buffer = buffers[i].data();
    bind.buffer_length = buffers[i].size();
    bind.is_null = reinterpret_cast<bool*>(&is_null_flags[i]);
    bind.length = &lengths[i];
  }
  // 将字段值数组与预处理句柄绑定
  if (mysql_stmt_bind_result(m_stmt.get(), binds.data())) {
    warn("[DatabaseConnection] Failed to bind result: %s",
         mysql_stmt_error(m_stmt.get()));
    return result;
  }

  for (int i = 0; i < row_count; ++i) {
    int status = mysql_stmt_fetch(m_stmt.get());
    if (status == MYSQL_NO_DATA) break;
    if (status == 1 || status == MYSQL_DATA_TRUNCATED) {
      warn(
          "[DatabaseConnection] Error or truncation while fetching result row");
      break;
    }
    json row;
    for (int j = 0; j < column_count; ++j) {
      const char* col_name = fields[j].name;
      if (is_null_flags[j]) {
        row[col_name] = nullptr;
        continue;
      }
      switch (fields[j].type) {
        case MYSQL_TYPE_TINY:  // 1 字节整数（通常是 bool 或 tinyint）
          row[col_name] =
              static_cast<int>(*reinterpret_cast<int8_t*>(buffers[j].data()));
          break;
        case MYSQL_TYPE_SHORT:  // 2 字节整数（smallint）
          row[col_name] =
              static_cast<int>(*reinterpret_cast<int16_t*>(buffers[j].data()));
          break;
        case MYSQL_TYPE_LONG:  // 4 字节整数（int）
          row[col_name] = *reinterpret_cast<int32_t*>(buffers[j].data());
          break;
        case MYSQL_TYPE_LONGLONG:  // 8 字节整数（bigint）
          row[col_name] = *reinterpret_cast<int64_t*>(buffers[j].data());
          break;
        case MYSQL_TYPE_FLOAT:  // 单精度
          row[col_name] = *reinterpret_cast<float*>(buffers[j].data());
          break;
        case MYSQL_TYPE_DOUBLE:  // 双精度
          row[col_name] = *reinterpret_cast<double*>(buffers[j].data());
          break;
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_BIT:
        case MYSQL_TYPE_ENUM:
        case MYSQL_TYPE_SET:
          row[col_name] = std::string(buffers[j].data(), lengths[j]);
          break;
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
          row[col_name] = std::string(buffers[j].data(),
                                      lengths[j]);  // 日期时间类也可以这样读
          break;
        case MYSQL_TYPE_NULL:
          row[col_name] = nullptr;
          break;
        default: {
          warn("[DatabaseConnection] Unhandled MySQL type for column %s: %d",
               col_name, fields[j].type);
          row[col_name] = "[unsupported type]";
          break;
        }
      }
    }
    result.emplace_back(std::move(row));
  }
  return result;
}

void DatabaseConnection::refresh() { m_idle_time = steady_clock::now(); }

long long DatabaseConnection::idle_time() {
  return duration_cast<milliseconds>(steady_clock::now() - m_idle_time).count();
}

bool DatabaseConnection::prepare(const std::string& sql) {
  if (sql.empty()) {
    warn("[DatabaseConnection] SQL statement is empty");
    return false;
  }

  // 如果之前已经准备过语句，先释放旧的语句句柄，避免内存泄漏
  if (m_stmt != nullptr) {
    m_stmt.reset();
  }

  // 使用当前数据库连接初始化一个新的 MYSQL_STMT 结构体（预处理语句句柄）
  m_stmt.reset(mysql_stmt_init(m_conn.get()));
  if (m_stmt == nullptr) {
    // 初始化失败，一般是内存不足或连接非法
    warn("[DatabaseConnection] Failed to init MYSQL_STMT");
    return false;
  }

  // 调用 mysql_stmt_prepare 编译 SQL 语句，准备后续绑定和执行
  if (mysql_stmt_prepare(m_stmt.get(), sql.c_str(), sql.size())) {
    // 编译失败则输出错误信息，通常是 SQL 语法错误或连接异常
    warn("[DatabaseConnection] Failed to prepare SQL: %s\nError: %s",
         sql.c_str(), mysql_stmt_error(m_stmt.get()));
    m_stmt.reset();  // 释放句柄，避免悬空
    return false;
  }

  debug("[DatabaseConnection] Prepare SQL is successful!");
  return true;  // 语句成功准备，可以绑定参数并执行
}

bool DatabaseConnection::execute() {
  // 释放以前的结果集
  m_result.reset();

  // 执行预处理语句
  if (mysql_stmt_execute(m_stmt.get())) {
    warn("[DatabaseConnection] Failed to execute prepared statement: %s",
         mysql_stmt_error(m_stmt.get()));
    return false;
  }

  // 判断是否为 SELECT 类型的语句：有返回字段说明是查询
  MYSQL_RES* meta_result = mysql_stmt_result_metadata(m_stmt.get());
  if (meta_result) {
    if (mysql_stmt_store_result(m_stmt.get()) != 0) {
      warn("[DatabaseConnection] Failed to store prepared result set: %s",
           mysql_stmt_error(m_stmt.get()));
      mysql_free_result(meta_result);
      return false;
    }

    m_result.reset(meta_result);
  }

  return true;
}

bool DatabaseConnection::bind_internal(size_t index) { return true; }

void DatabaseConnection::mysql_deleter(MYSQL* conn) {
  if (conn != nullptr) {
    mysql_close(conn);
  }
}

void DatabaseConnection::mysql_res_deleter(MYSQL_RES* result) {
  if (result != nullptr) {
    mysql_free_result(result);
  }
}

void DatabaseConnection::mysql_stmt_deleter(MYSQL_STMT* stmt) {
  if (stmt != nullptr) {
    mysql_stmt_close(stmt);
  }
}

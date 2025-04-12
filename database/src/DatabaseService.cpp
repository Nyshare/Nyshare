#include "DatabaseService.h"

DatabaseService::database_status DatabaseService::login(
    const std::string& username, const std::string& password) {
  auto database = DatabaseConnectionPool::getInstance().acquire();
  // 1、检查用户名是否注册
  if (!database->query("SELECT id FROM users WHERE username = ?", username)) {
    return database_status::database_error;
  }
  if (database->empty()) {
    return database_status::user_not_registered;
  }

  // 2、检查密码是否正确
  if (!database->query("SELECT id FROM users WHERE username = ? AND "
                       "password = ?",
                       username, password)) {
    return database_error;
  }
  if (database->empty()) {
    return database_status::incorrect_password;
  }

  // 3、登录成功
  return database_status::login_successful;
}

DatabaseService::database_status DatabaseService::signup(
    const std::string& email, const std::string& username,
    const std::string& password) {
  auto database = DatabaseConnectionPool::getInstance().acquire();
  // 1、检查邮箱是否已注册
  if (!database->query("SELECT id FROM users WHERE email = ?", email)) {
    return database_status::database_error;
  }
  if (!database->empty()) {
    return database_status::email_already_registered;
  }
  // 2、检查用户名是否已被注册
  if (!database->query("SELECT id FROM users WHERE username = ?", username)) {
    return database_status::database_error;
  }
  if (!database->empty()) {
    return database_status::username_already_taken;
  }
  // 3、注册成功，向数据库添加新用户
  if (!database->query(
          "INSERT INTO users (email, username, password) VALUES (?, ?, ?)",
          email, username, password)) {
    return database_status::database_error;
  }
  return database_status::registration_successful;
}

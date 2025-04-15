#include "DatabaseService.h"

#include "DatabaseUtil.h"

DatabaseService::database_status DatabaseService::login(
    const std::string& username, const std::string& password) {
  auto database = DatabaseConnectionPool::instance().acquire();
  // 1、检查用户名是否注册
  if (!database->query("SELECT id, password FROM users WHERE username = ?",
                       username)) {
    return database_status::database_error;
  }
  if (database->empty()) {
    return database_status::user_not_registered;
  }

  // 2、检查密码是否正确
  json result = database->result().at(0);
  const auto& storedPassword = result.value("password", "");
  if (!DatabaseUtil::checkPassword(password, storedPassword)) {
    return database_status::incorrect_password;
  }
  // 3、登录成功
  return database_status::login_successful;
}

DatabaseService::database_status DatabaseService::signup(
    const std::string& email, const std::string& username,
    const std::string& password) {
  auto database = DatabaseConnectionPool::instance().acquire();
  // 检查邮箱是否已注册
  if (!database->query("SELECT id FROM users WHERE email = ?", email)) {
    return database_status::database_error;
  }
  if (!database->empty()) {
    return database_status::email_already_registered;
  }
  // 检查用户名是否已被注册
  if (!database->query("SELECT id FROM users WHERE username = ?", username)) {
    return database_status::database_error;
  }
  if (!database->empty()) {
    return database_status::username_already_taken;
  }

  // 加密用户密码
  std::string storePassword = DatabaseUtil::encryptPassword(password);
  // 注册成功，向数据库添加新用户
  if (!database->query(
          "INSERT INTO users (email, username, password) VALUES (?, ?, ?)",
          email, username, storePassword)) {
    return database_status::database_error;
  }
  return database_status::registration_successful;
}

DatabaseService::database_status DatabaseService::resetPassword(
    const std::string& email, const std::string& password) {
  // 获取数据库连接
  auto database = DatabaseConnectionPool::instance().acquire();

  // 检查用户是否已注册
  if (!database->query("SELECT id FROM users WHERE username = ?", email)) {
    return database_status::database_error;
  }
  if (database->empty()) {
    return database_status::email_not_registered;
  }

  // 加密用户密码
  std::string storePassword = DatabaseUtil::encryptPassword(password);

  // 将密文密码存入数据库
  if (!database->query("UPDATE users SET password = ? WHERE email = ?",
                       password, email)) {
    return database_status::database_error;
  }
  return database_status::password_reset_successful;
}

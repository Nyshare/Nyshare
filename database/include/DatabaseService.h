#pragma once

#include "DatabaseConnectionPool.h"

class DatabaseService {
 public:
  enum database_status {
    database_error = 0,       // 数据库异常
    user_not_registered = 1,  // 用户尚未注册
    incorrect_password = 2,   // 密码错误
    login_successful = 3,     // 登录成功

    email_already_registered = 4,  // 邮箱已注册
    username_already_taken = 5,    // 用户名已被注册
    registration_successful = 6,   // 注册成功

    email_not_registered = 7,       // 邮箱尚未注册
    password_reset_successful = 8,  // 密码重置成功
  };

  // 处理登录
  static database_status login(const std::string &username,
                               const std::string &password);

  // 处理注册
  static database_status signup(const std::string &email,
                                const std::string &username,
                                const std::string &password);
  // 重置密码
  static database_status resetPassword(const std::string &username,
                                       const std::string &password);
};
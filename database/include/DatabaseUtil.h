#pragma once

#include <jwt-cpp/jwt.h>

#include <string>
#include <vector>

class DatabaseUtil {
 public:
  // 获取用户密码的Hash值
  static std::string encryptPassword(const std::string& password);

  // 比对用户明文密码与数据库Hash密码
  static bool checkPassword(const std::string& password,
                            const std::string& stored);

 private:
  // 编码为 hex 字符串
  static std::string toHex(const unsigned char* data, size_t len);
  // 从 hex 字符串解析为原始数据
  static std::vector<unsigned char> fromHex(const std::string& hex);
};
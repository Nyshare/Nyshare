#pragma once

#include <sw/redis++/redis.h>

#include <memory>

class Cacher {
 public:
  static Cacher& instance();

  // 保存验证码
  bool save_verification_code(const std::string& email,
                              const std::string& verification_code);
  bool check_verification_code(const std::string& email,
                               const std::string& verification_code);

 private:
  Cacher();
  Cacher(const Cacher& object) = delete;
  Cacher& operator=(const Cacher& object) = delete;
  std::unique_ptr<sw::redis::Redis> m_redis;
};
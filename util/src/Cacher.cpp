#include "Cacher.h"

#include "Logger.h"

using namespace sw::redis;

Cacher& Cacher::instance() {
  static Cacher instance;
  return instance;
}

bool Cacher::save_verification_code(const std::string& email,
                                    const std::string& verification_code) {
  try {
    if (m_redis->exists(email)) {
      return false;
    }
    m_redis->set(email, verification_code, std::chrono::seconds(600));
    return true;
  } catch (const Error& e) {
    warn("[Cacher] Redis failed save verification code:%s", e.what());
    return false;
  }
}

bool Cacher::check_verification_code(const std::string& email,
                                     const std::string& verification_code) {
  try {
    auto value = m_redis->get(email);
    if (value && *value == verification_code) {
      m_redis->del(email);  // 验证成功后删除，防止重复使用
      return true;
    }
    return false;
  } catch (const Error& e) {
    warn("[Cacher] Redis failed check verification code:%s", e.what());
    return false;
  }
}

Cacher::Cacher() {
  try {
    m_redis = std::make_unique<Redis>("tcp://127.0.0.1:6379");
  } catch (const Error& e) {
    fatal("[Cacher] Redis failed initialize: %s", e.what());
    throw std::logic_error("[Cacher] Redis failed initialize");
  }
}

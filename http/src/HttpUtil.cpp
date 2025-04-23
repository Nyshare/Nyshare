#include "HttpUtil.h"

#include <jwt-cpp/jwt.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <nlohmann/json.hpp>
#include <random>

#include "Logger.h"

const std::string secret_key =
    "b157Y0NvqmpO2VVG9zsLD+"
    "w1G8gnPRCIyvRu9cEMGfbLf5knnnRuedf1zRQFRF7wQyZDUb6LJw1WDV/LGIDqVg ==";

bool HttpUtil::legalUsername(const std::string& username) {
  if (username.empty() || username.size() > 32) {
    warn("用户名长度错误:%s", username);
    return false;
  }
  for (const auto& ch : username) {
    if (isspace(ch) || iscntrl(ch)) {
      warn("用户名含有空格或不可见字符:%s", username);
      return false;
    }
  }
  return true;
}

bool HttpUtil::legalPassword(const std::string& password) {
  if (password.size() < 6 || password.size() > 32) {
    warn("密码长度错误:%s", password);
    return false;
  }

  // 必须包含数字、大写字母、小写字母、特殊符号各一个
  bool hasDigit = false;
  bool hasUpper = false;
  bool hasLower = false;
  bool hasSpecial = false;

  for (const auto& ch : password) {
    if (ch < 33 || ch > 126) {  // 只允许ASCII中的可打印字符
      warn("密码含有不可见字符:%s", password);
      return false;
    }
    if (isdigit(ch)) {
      hasDigit = true;
    } else if (isupper(ch)) {
      hasUpper = true;
    } else if (islower(ch)) {
      hasLower = true;
    } else {
      hasSpecial = true;
    }
  }

  return hasDigit && hasUpper && hasLower && hasSpecial;
}

bool HttpUtil::legalEmail(const std::string& email) {
  // 合法邮箱长度一般不超过 254 个字符
  if (email.empty() || email.size() > 254) {
    return false;
  }

  // 找到 @ 符号的位置
  auto atPos = email.find('@');
  if (atPos == std::string::npos || atPos == 0 || atPos == email.size() - 1) {
    return false;  // 必须包含 @，且 @ 不在开头或结尾
  }

  std::string localPart = email.substr(0, atPos);
  std::string domainPart = email.substr(atPos + 1);

  // 检查 localPart 和 domainPart 是否为空
  if (localPart.empty() || domainPart.empty()) {
    return false;
  }

  // localPart 允许：字母数字和部分特殊字符 ._%+-
  for (const auto& ch : localPart) {
    if (!isalnum(ch) && ch != '.' && ch != '_' && ch != '%' && ch != '+' &&
        ch != '-') {
      return false;
    }
  }

  // domainPart 必须至少包含一个点 . 且不能以点开头或结尾
  auto dotPos = domainPart.find('.');
  if (dotPos == std::string::npos || domainPart.front() == '.' ||
      domainPart.back() == '.') {
    return false;
  }

  // domainPart 允许：字母数字和 -
  for (const auto& ch : domainPart) {
    if (!isalnum(ch) && ch != '.' && ch != '-') {
      return false;
    }
  }

  return true;
}

bool HttpUtil::legalVerificationCode(const std::string& verificationCode) {
  if (verificationCode.size() != 6) {
    return false;
  }
  for (const auto& ch : verificationCode) {
    if (!isdigit(ch)) {
      return false;
    }
  }
  return true;
}

void HttpUtil::setjsonResponse(HttpResponse& httpResponse,
                               const json& jsonResponse) {
  std::string body = jsonResponse.dump();
  httpResponse.body(body)
      .content_type("application/json")
      .content_length(body.size())
      .set("Connection", "keep-alive");
}

std::string HttpUtil::generateVerificationCode() {
  static std::random_device rd;   // 用于获取随机种子
  static std::mt19937 gen(rd());  // 高质量随机数生成器
  static std::uniform_int_distribution<> dis(0, 9);

  std::string verificationCode;
  for (int i = 0; i < 6; ++i) {
    verificationCode += std::to_string(dis(gen));
  }
  return verificationCode;
}

void HttpUtil::setSuccessResponse(HttpResponse& res, json& jsonResponse,
                                  const std::string& msg) {
  jsonResponse["success"] = true;
  jsonResponse["message"] = msg;
  res.status(HttpResponse::OK);
  setjsonResponse(res, jsonResponse);
}

void HttpUtil::setFailResponse(HttpResponse& res, HttpResponse::STATUS code,
                               const std::string& msg) {
  json json;
  json["success"] = false;
  json["message"] = msg;
  res.status(code);
  setjsonResponse(res, json);
}

json HttpUtil::parseRequestBody(const std::string& body) {
  json body_json;
  try {
    body_json = json::parse(body);
  } catch (const std::exception& e) {
    warn("[HttpUtil] parse json error:%s", body.c_str());
  }
  return body_json;
}

std::string HttpUtil::generate_token(int user_id) {
  auto token = jwt::create()
                   .set_issuer("Nyshare")
                   .set_audience("nyshare-web")
                   .set_subject(std::to_string(user_id))  // 用户ID
                   .set_issued_at(std::chrono::system_clock::now())
                   .set_expires_at(std::chrono::system_clock::now() +
                                   std::chrono::hours{8760})  // 有效期一年
                   .sign(jwt::algorithm::hs256{secret_key});  // 密钥越复杂越好

  return token;
}

bool HttpUtil::verify_token(const std::string& token_str) {
  try {
    auto decoded = jwt::decode(token_str);

    jwt::verify()
        .allow_algorithm(jwt::algorithm::hs256{secret_key})
        .with_issuer("Nyshare")
        .with_audience("nyshare-web")
        .verify(decoded);
    return true;
  } catch (const std::exception& e) {
    warn("Token failed verify: %s", e.what());
    return false;
  }
}

std::string HttpUtil::extract_token(const HttpRequest& request) {
  static const std::string empty_string;
  std::string auth = request[HttpRequest::AUTHORIZATION];
  if (auth.empty()) {
    return empty_string;
  }
  std::istringstream authStream(auth);
  std::string bearer, token;
  authStream >> bearer >> token;
  return token;
}

int HttpUtil::extract_user_id(const std::string& token_str) {
  try {
    auto decoded = jwt::decode(token_str);
    return std::stoi(decoded.get_subject());
  } catch (const std::exception& e) {
    debug("token = %s", token_str.c_str());
    warn("Token failed verify: %s", e.what());
    return -1;
  }
}

std::string HttpUtil::generate_unique_code() {
  // 获取当前时间戳
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

  // 生成随机数
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(100000, 999999);
  int random_number = dis(gen);

  // 组合时间戳和随机数
  std::stringstream ss;
  ss << milliseconds << "_" << random_number;
  return ss.str();
}

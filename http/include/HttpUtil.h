#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "HttpRequest.h"
#include "HttpResponse.h"

using json = nlohmann::json;

class HttpUtil {
 public:
  // 检查用户名合法性
  static bool legalUsername(const std::string& username);
  // 检查密码合法性
  static bool legalPassword(const std::string& password);
  // 检查邮箱合法性
  static bool legalEmail(const std::string& email);
  // 检查验证码合法性
  static bool legalVerificationCode(const std::string& verificationCode);
  // 统一设置json格式的响应
  static void setjsonResponse(HttpResponse& httpResponse,
                              const json& jsonResponse);
  // 随机生成验证码
  static std::string generateVerificationCode();
  // 统一设置true响应
  static void setSuccessResponse(HttpResponse& res, json& jsonResponse,
                                 const std::string& msg);
  // 统一设置false响应
  static void setFailResponse(HttpResponse& res, HttpResponse::StatusCode code,
                              const std::string& msg);
  // 将请求体解析为json格式，失败返回null类型的json
  static json parseRequestBody(const std::string& body);
  // 生成token
  static std::string generate_token(const std::string username);
  // 验证tokrn
  static std::string verify_token(const std::string& token_str);
  //  声明纯虚析构防止实例化
  virtual ~HttpUtil() = 0;
};
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
  static std::string generate_token(int user_id);
  // 验证token
  static bool verify_token(const std::string& token_str);
  // 从请求中提取token
  static std::string extract_token(const HttpRequest& request);

  // 从Token中提取用户ID
  static int extract_user_id(const std::string& token_str);

  // 生成唯一的作品码
  static std::string generate_unique_code();
};
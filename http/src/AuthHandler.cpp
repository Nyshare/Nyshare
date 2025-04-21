#include "AuthHandler.h"

#include <nlohmann/json.hpp>
#include <sstream>

#include "Cacher.h"
#include "DatabaseService.h"
#include "Emailer.h"
#include "HttpUtil.h"

using json = nlohmann::json;
using status = DatabaseService::database_status;

void AuthHandler::login(HttpRequest& httpRequest, HttpResponse& httpResponse) {
  json loginJson;
  // 从请求体中解析出用户名和密码
  json requestJson = HttpUtil::parseRequestBody(httpRequest.getBody());
  if (requestJson.type() == json::value_t::null) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::BadRequest,
                              "请求体格式错误");
  }

  std::string username = requestJson.value("username", "");
  std::string password = requestJson.value("password", "");

  // 检验用户名和密码的有效性
  if (!HttpUtil::legalUsername(username) ||
      !HttpUtil::legalPassword(password)) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::BadRequest,
                              "用户名或密码格式错误");
    return;
  }

  int user_id = 0;
  switch (DatabaseService::login(username, password, user_id)) {
    case status::user_not_registered: {
      HttpUtil::HttpUtil::setFailResponse(
          httpResponse, HttpResponse::BadRequest, "用户未注册");
      break;
    }
    case status::incorrect_password: {
      HttpUtil::HttpUtil::setFailResponse(
          httpResponse, HttpResponse::Unauthorized, "密码错误");
      break;
    }
    case status::login_successful: {
      loginJson["token"] = HttpUtil::generate_token(user_id);
      // debug("token: %s", loginJson["token"].dump().c_str());
      HttpUtil::setSuccessResponse(httpResponse, loginJson, "登录成功");
      break;
    }
    default: {
      loginJson["success"] = false;
      loginJson["message"] = "服务器异常";
      httpResponse.setStatusCode(HttpResponse::InternalServerError);
      break;
    }
  }
}

void AuthHandler::signup(HttpRequest& httpRequest, HttpResponse& httpResponse) {
  json jsonSignup;
  // 解析注册参数
  json requestJson =
      HttpUtil::HttpUtil::parseRequestBody(httpRequest.getBody());
  if (requestJson.type() == json::value_t::null) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::InternalServerError,
                              "请求格式错误");
    return;
  }

  std::string email = requestJson.value("email", "");
  std::string verificationCode = requestJson.value("verification_code", "");
  std::string username = requestJson.value("username", "");
  std::string password = requestJson.value("password", "");

  if (!HttpUtil::legalEmail(email) ||
      !HttpUtil::legalVerificationCode(verificationCode) ||
      !HttpUtil::legalUsername(username) ||
      !HttpUtil::legalPassword(password)) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::BadRequest,
                              "格式错误");
    return;
  }

  if (!Cacher::instance().check_verification_code(email, verificationCode)) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::Unauthorized,
                              "验证码无效");
    return;
  }

  switch (DatabaseService::signup(email, username, password)) {
    case DatabaseService::email_already_registered: {
      HttpUtil::setFailResponse(httpResponse, HttpResponse::Unauthorized,
                                "邮箱已注册");
      break;
    }
    case DatabaseService::username_already_taken: {
      HttpUtil::setFailResponse(httpResponse, HttpResponse::Unauthorized,
                                "用户名已注册");
      break;
    }
    case DatabaseService::registration_successful: {
      HttpUtil::setSuccessResponse(httpResponse, jsonSignup, "注册成功");
      break;
    }
    default: {
      HttpUtil::setFailResponse(httpResponse, HttpResponse::InternalServerError,
                                "服务器异常");
      break;
    }
  }
}

void AuthHandler::sendVerificationCode(HttpRequest& httpRequest,
                                       HttpResponse& httpResponse) {
  json requestJson = HttpUtil::parseRequestBody(httpRequest.getBody());
  if (requestJson.type() == json::value_t::null) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::InternalServerError,
                              "请求体格式错误");
    return;
  }
  std::string email = requestJson["email"];
  if (!HttpUtil::legalEmail(email)) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::BadRequest,
                              "邮箱格式错误");
    return;
  }

  // 生成验证码
  std::string verificationCode = HttpUtil::generateVerificationCode();

  // 将验证码存入缓存
  if (!Cacher::instance().save_verification_code(email, verificationCode)) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::BadRequest,
                              "访问频繁");
    return;
  }

  // 发送验证码
  if (!EmailSender::instance().sendVerificationCode(email, verificationCode)) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::InternalServerError,
                              "发送失败");
    return;
  }

  json responseJson;
  HttpUtil::setSuccessResponse(httpResponse, responseJson, "发送成功");
}

void AuthHandler::resetPassword(HttpRequest& httpRequest,
                                HttpResponse& httpResponse) {
  json jsonResponse;
  // 解析注册参数
  json requestJson = HttpUtil::parseRequestBody(httpRequest.getBody());
  if (jsonResponse.type() == json::value_t::null) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::InternalServerError,
                              "服务器异常");
    return;
  }
  std::string email = requestJson["email"];
  std::string verificationCode = requestJson["verification_code"];
  std::string password = requestJson["password"];
  // 检验输入合法性
  if (!HttpUtil::legalEmail(email) ||
      !HttpUtil::legalVerificationCode(verificationCode) ||
      !HttpUtil::legalPassword(password)) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::BadRequest,
                              "格式错误");
    return;
  }

  // 检验验证码的有效性
  if (!Cacher::instance().check_verification_code(email, verificationCode)) {
    HttpUtil::setFailResponse(httpResponse, HttpResponse::Unauthorized,
                              "验证码无效");
    return;
  }

  // 调用数据库接口
  switch (DatabaseService::resetPassword(email, password)) {
    case DatabaseService::email_not_registered: {
      HttpUtil::setFailResponse(httpResponse, HttpResponse::Unauthorized,
                                "邮箱尚未注册");
      break;
    }
    case DatabaseService::password_reset_successful: {
      HttpUtil::setSuccessResponse(httpResponse, jsonResponse, "密码修改成功");
      break;
    }
    default: {
      HttpUtil::setFailResponse(httpResponse, HttpResponse::InternalServerError,
                                "服务器异常");
      break;
    }
  }
}

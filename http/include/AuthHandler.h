#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

class AuthHandler {
 public:
  // 登录
  static void login(HttpRequest& httpRequest, HttpResponse& httpResponse);
  // 注册
  static void signup(HttpRequest& httpRequest, HttpResponse& httpResponse);
  // 发送验证码
  static void sendVerificationCode(HttpRequest& httpRequest,
                                   HttpResponse& httpResponse);
  // 重置密码
  static void resetPassword(HttpRequest& httpRequest,
                            HttpResponse& httpResponse);
};
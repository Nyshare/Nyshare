#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

class AuthHandler {
 public:
  // 登录
  static void login(HttpRequest& request, HttpResponse& response);
  // 注册
  static void signup(HttpRequest& request, HttpResponse& response);
  // 发送验证码
  static void sendVerificationCode(HttpRequest& request,
                                   HttpResponse& response);
  // 重置密码
  static void resetPassword(HttpRequest& request, HttpResponse& response);
};
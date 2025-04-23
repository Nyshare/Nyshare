#pragma once
#include <memory>
#include <string>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "MiddlewareChain.h"
#include "Router.h"

// 前向声明依赖类
class HttpRequest;
class HttpResponse;
class Router;
class MiddlewareChain;

class HttpHandler {
 public:
  HttpHandler(std::shared_ptr<MiddlewareChain> middlewareChain);  // 构造函数

  // 功能：处理原始的HTTP请求字符串，返回响应字符串
  // 参数：HTTP请求的原始字符串
  // 返回值：返回一个响应字符串，可直接发送至客户端
  std::string handleHttpRequest(const std::string &rawRequest);

 private:
  std::unique_ptr<HttpRequest> _httpRequest;          // 存储HTTP请求的各个部分
  std::unique_ptr<HttpResponse> _httpResponse;        // 生成HTTP响应
  std::shared_ptr<MiddlewareChain> _middlewareChain;  // 保存并执行中间件链
};

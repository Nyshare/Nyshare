#pragma once

#include <functional>

#include "HttpRequest.h"
#include "HttpResponse.h"

class Router {
 public:
  // 定义路由函数的类型
  using HandleFunc = std::function<void(HttpRequest &, HttpResponse &)>;

  // 注册路由函数
  void addRoute(HttpRequest::METHOD method, const std::string &path,
                HandleFunc handler);

  // 查找并执行路由处理器,查找并执行成功返回true,失败返回false
  bool route(HttpRequest &httpRequest, HttpResponse &httpResponse) const;

 private:
  // 路由表：方法->路径->处理函数
  std::unordered_map<HttpRequest::METHOD,
                     std::unordered_map<std::string, HandleFunc>>
      _routes;
};
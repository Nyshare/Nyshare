#pragma once

#include <string>

#include "Middleware.h"
class StaticFileMiddleware : public Middleware {
 public:
  explicit StaticFileMiddleware(const std::string& staticDir);

  // 读取文件内容
  void handleRequest(HttpRequest& httpRequest, HttpResponse& httpResponse,
                     std::function<void()> next) override;
  void handleResponse(HttpRequest& httpRequest,
                      HttpResponse& httpResponse) override;

 private:
  std::string _staticDir;
};
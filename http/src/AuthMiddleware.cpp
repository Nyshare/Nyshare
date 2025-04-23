#include "AuthMiddleware.h"

#include <sstream>
#include <unordered_set>

#include "Cacher.h"
#include "HttpUtil.h"

void AuthMiddleware::handleRequest(HttpRequest& httpRequest,
                                   HttpResponse& httpResponse,
                                   std::function<void()> next) {
  static const std::unordered_set<std::string> protectedPaths = {
      "/api/upload_post", "/upload.html"};
  if (protectedPaths.find(httpRequest.url()) == protectedPaths.end()) {
    next();
    return;
  }
  json requestJson;
  // 获取包含 token 的响应头
  std::string auth = httpRequest[HttpRequest::AUTHORIZATION];
  if (auth.empty()) {
    httpResponse.set(HttpResponse::LOCATION, "/login.html");
    HttpUtil::setFailResponse(httpResponse, HttpResponse::Found, " token无效");
    return;
  }

  // 从请求中提取 token
  std::string token = HttpUtil::extract_token(httpRequest);
  if (token.empty()) {
    httpResponse.set(HttpResponse::LOCATION, "/login.html");
    HttpUtil::setFailResponse(httpResponse, HttpResponse::Found, " token无效");
    return;
  }
  // 验证 token
  if (!HttpUtil::verify_token(token)) {
    httpResponse.set(HttpResponse::LOCATION, "/login.html");
    HttpUtil::setFailResponse(httpResponse, HttpResponse::Found, " token无效");
    return;
  }
  next();
}

void AuthMiddleware::handleResponse(HttpRequest& httpRequest,
                                    HttpResponse& httpResponse) {}

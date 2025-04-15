#include "AuthMiddleware.h"

#include <sstream>
#include <unordered_set>

#include "Cacher.h"
#include "HttpUtil.h"

void AuthMiddleware::handleRequest(HttpRequest& httpRequest,
                                   HttpResponse& httpResponse,
                                   std::function<void()> next) {
  static const std::unordered_set<std::string> protectedPaths = {
      "/profile.html"};
  if (protectedPaths.find(httpRequest.getPath()) == protectedPaths.end()) {
    next();
    return;
  }
  json requestJson;
  // 获取包含 token 的响应头
  std::string auth = httpRequest.getHeader("Authorization");
  if (auth.empty()) {
    httpResponse.addHeader("Location", "/signin.html");
    HttpUtil::setFailResponse(httpResponse, HttpResponse::Found, " token无效");
    return;
  }

  // 从响应头提取 token
  std::istringstream authStream(auth);
  std::string bearer, token;
  authStream >> bearer >> token;

  // 从数据库查询 token 的有效性
  if (HttpUtil::verify_token(token).empty()) {
    httpResponse.addHeader("Location", "/signin.html");
    HttpUtil::setFailResponse(httpResponse, HttpResponse::Found, "token无效");
    return;
  }
  next();
}

void AuthMiddleware::handleResponse(HttpRequest& httpRequest,
                                    HttpResponse& httpResponse) {}

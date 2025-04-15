#include "HttpHandler.h"

#include <iostream>

#include "AuthHandler.h"
#include "AuthMiddleware.h"
#include "ErrorMiddleware.h"
#include "LogMiddleware.h"
#include "Logger.h"
#include "RouterMiddleware.h"
#include "StaticFileMiddleware.h"

HttpHandler::HttpHandler()
    : _httpRequest(std::make_unique<HttpRequest>()),
      _httpResponse(std::make_unique<HttpResponse>()),
      _router(std::make_unique<Router>()),
      _middlewareChain(std::make_unique<MiddlewareChain>()) {
  // 默认中间件链：日志 -> 权限校验 -> 静态文件 -> 错误处理 -> 路由器
  _middlewareChain->addMiddleware(std::make_unique<LogMiddleware>());
  _middlewareChain->addMiddleware(std::make_unique<AuthMiddleware>());
  _middlewareChain->addMiddleware(
      std::make_unique<StaticFileMiddleware>("../public/static"));
  _middlewareChain->addMiddleware(std::make_unique<ErrorMiddleware>());
  _middlewareChain->addMiddleware(std::make_unique<RouterMiddleware>(*_router));
  _router->addRoute(HttpRequest::POST, "/login",
                    [](HttpRequest& req, HttpResponse& res) {
                      AuthHandler::login(req, res);
                    });
  _router->addRoute(HttpRequest::POST, "/send_verification_code",
                    [](HttpRequest& req, HttpResponse& res) {
                      AuthHandler::sendVerificationCode(req, res);
                    });
  _router->addRoute(HttpRequest::POST, "/signup",
                    [](HttpRequest& req, HttpResponse& res) {
                      AuthHandler::signup(req, res);
                    });
  _router->addRoute(HttpRequest::POST, "/reset_password",
                    [](HttpRequest& req, HttpResponse& res) {
                      AuthHandler::resetPassword(req, res);
                    });
}

std::string HttpHandler::handleHttpRequest(const std::string& rawRequest) {
  // 1、将原始 HTTP 请求字符串解析成 HttpRequest 对象
  if (!_httpRequest->parse(rawRequest)) {
    _httpResponse->setStatusCode(HttpResponse::BadRequest);
    _httpResponse->setBody("Invalid HTTP Request");
    return _httpResponse->toString();
  }
  // 中间件链处理请求
  _middlewareChain->handle(*_httpRequest, *_httpResponse);
  // 返回最终响应
  return _httpResponse->toString();
}

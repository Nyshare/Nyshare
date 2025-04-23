#include "HttpHandlerPool.h"

#include <memory>

#include "AuthHandler.h"
#include "AuthMiddleware.h"
#include "ErrorMiddleware.h"
#include "LogMiddleware.h"
#include "Logger.h"
#include "PostsHandler.h"
#include "RouterMiddleware.h"
#include "StaticFileMiddleware.h"

HttpHandlerPool& HttpHandlerPool::instance() {
  static HttpHandlerPool instance;
  return instance;
}

std::shared_ptr<HttpHandler> HttpHandlerPool::acquire() {
  std::lock_guard<std::mutex> lock(mutex_);
  while (handlerPool_.empty()) {
    if (count_ >= MAX_POOL_SIZE) {
      // 池满了，等待
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    handlerPool_.push(std::make_shared<HttpHandler>(middlewareChain_));
    count_++;
  }

  auto handler = handlerPool_.front();
  handlerPool_.pop();

  std::shared_ptr<HttpHandler> handlerPtr(
      handler.get(),                   // 对外暴露裸指针
      [this, handler](HttpHandler*) {  // 自定义 deleter
        std::lock_guard<std::mutex> lock(mutex_);
        handlerPool_.push(handler);  // 回收进池
      });
  return handlerPtr;
}

HttpHandlerPool::HttpHandlerPool()
    : router_(std::make_shared<Router>()),
      middlewareChain_(std::make_shared<MiddlewareChain>()),
      count_(0) {
  // 默认中间件链：日志 -> 权限校验 -> API -> 静态文件 -> 错误处理
  middlewareChain_->addMiddleware(std::make_unique<ErrorMiddleware>());
  middlewareChain_->addMiddleware(std::make_unique<LogMiddleware>());
  middlewareChain_->addMiddleware(std::make_unique<AuthMiddleware>());
  middlewareChain_->addMiddleware(std::make_unique<RouterMiddleware>(router_));
  middlewareChain_->addMiddleware(
      std::make_unique<StaticFileMiddleware>("../static"));
  router_->addRoute(HttpRequest::POST, "/api/login",
                    [](HttpRequest& req, HttpResponse& res) {
                      AuthHandler::login(req, res);
                    });
  router_->addRoute(HttpRequest::POST, "/api/send_verification_code",
                    [](HttpRequest& req, HttpResponse& res) {
                      AuthHandler::sendVerificationCode(req, res);
                    });
  router_->addRoute(HttpRequest::POST, "/api/signup",
                    [](HttpRequest& req, HttpResponse& res) {
                      AuthHandler::signup(req, res);
                    });
  router_->addRoute(HttpRequest::POST, "/api/reset_password",
                    [](HttpRequest& req, HttpResponse& res) {
                      AuthHandler::resetPassword(req, res);
                    });
  router_->addRoute(HttpRequest::GET, "/api/get_posts",
                    [](HttpRequest& req, HttpResponse& res) {
                      // 处理函数
                      PostsHandler::handleGetPosts(req, res);
                    });
  router_->addRoute(HttpRequest::POST, "/api/upload_post",
                    [](HttpRequest& req, HttpResponse& res) {
                      // 处理函数
                      PostsHandler::handleUploadPost(req, res);
                    });
}
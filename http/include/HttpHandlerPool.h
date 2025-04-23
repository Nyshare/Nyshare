#pragma once

#include <mutex>
#include <queue>

#include "HttpHandler.h"
#include "MiddlewareChain.h"
#include "Router.h"

class HttpHandlerPool {
 public:
  static HttpHandlerPool& instance();
  std::shared_ptr<HttpHandler> acquire();

 private:
  HttpHandlerPool();
  HttpHandlerPool(const HttpHandlerPool&) = delete;
  HttpHandlerPool& operator=(const HttpHandlerPool&) = delete;

 private:
  static const int MAX_POOL_SIZE = 10;
  std::queue<std::shared_ptr<HttpHandler>> handlerPool_;
  std::shared_ptr<MiddlewareChain> middlewareChain_;
  std::shared_ptr<Router> router_;
  std::mutex mutex_;
  int count_;
};
#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

class PostsHandler {
 public:
  // 处理拉去作品请求
  static void handleGetPosts(const HttpRequest& request,
                             HttpResponse& response);
  // 处理发布作品请求
  static void handleUploadPost(const HttpRequest& request,
                               HttpResponse& response);

 private:
};
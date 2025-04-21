#include "PostsHandler.h"

#include "DatabaseService.h"
#include "Filer.h"
#include "HttpUtil.h"
#include "Logger.h"

void PostsHandler::handleGetPosts(const HttpRequest& request,
                                  HttpResponse& response) {
  // 获取作品列表
  auto posts = DatabaseService::getPosts();
  if (posts.empty()) {
    HttpUtil::setFailResponse(response, HttpResponse::InternalServerError,
                              "获取作品失败");
    return;
  }
  json responseJson;
  responseJson["posts"] = posts;

  HttpUtil::setSuccessResponse(response, responseJson, "获取作品成功");
}

void PostsHandler::handleUploadPost(const HttpRequest& request,
                                    HttpResponse& response) {
  // 提取token
  std::string token = HttpUtil::extract_token(request);
  if (token.empty()) {
    HttpUtil::setFailResponse(response, HttpResponse::Unauthorized,
                              "token无效");
    return;
  }

  // 提取用户ID
  int user_id = HttpUtil::extract_user_id(token);
  if (user_id == -1) {
    HttpUtil::setFailResponse(response, HttpResponse::Unauthorized,
                              "token无效");
    debug("user_id: %d", user_id);
    return;
  }

  // 提取文件内容
  json requestJson = HttpUtil::parseRequestBody(request.getBody());
  if (requestJson.type() != json::value_t::object) {
    HttpUtil::setFailResponse(response, HttpResponse::BadRequest,
                              "请求体格式错误");
    return;
  }
  std::string text = requestJson.value("text", "");
  if (text.empty()) {
    HttpUtil::setFailResponse(response, HttpResponse::BadRequest,
                              "请求体格式错误");
    return;
  }

  // 获取作品到期时间
  std::string expire_at = requestJson.value("expire_at", "");
  if (expire_at.empty()) {
    HttpUtil::setFailResponse(response, HttpResponse::BadRequest,
                              "请求体格式错误");
    return;
  }

  // 生成作品码
  std::string unique_code = HttpUtil::generate_unique_code();

  std::string post_url = "/posts/" + unique_code;

  // 文件路径
  std::string file_path = Filer::getAbsolutePath("../static" + post_url);

  // 存入数据库
  if (!DatabaseService::uploadPost(user_id, post_url, expire_at)) {
    HttpUtil::setFailResponse(response, HttpResponse::InternalServerError,
                              "服务器繁忙");
    return;
  }

  // 写入文件
  if (!Filer::writeFile(file_path, text)) {
    HttpUtil::setFailResponse(response, HttpResponse::InternalServerError,
                              "服务器繁忙");
    return;
  }

  // 设置响应
  json responseJson;
  HttpUtil::setSuccessResponse(response, responseJson, "上传作品成功");
}

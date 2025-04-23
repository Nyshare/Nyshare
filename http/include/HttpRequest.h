#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class HttpRequest {
 public:
  // 请求方法枚举
  enum METHOD {
    GET = 0,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    UNKNOWN
  };

  // 请求头枚举
  enum HEADER {
    HOST = 0,
    USER_AGENT,
    ACCEPT,
    ACCEPT_LANGUAGE,
    ACCEPT_ENCODING,
    CONNECTION,
    CONTENT_TYPE,
    CONTENT_LENGTH,
    COOKIE,
    AUTHORIZATION,
  };

  HttpRequest();
  HttpRequest(const std::string &rawRequest);

  // 获取HTTP请求方法
  METHOD method() const;
  const std::string &methodStr() const;

  // 获取HTTP请求路径
  const std::string &url() const;

  // 获取HTTP请求参数
  const std::string &param(const std::string &key) const;

  // 获取HTTP请求版本
  const std::string &version() const;

  // 检查是否包含某个请求头
  bool has_header(HEADER key) const;
  bool has_header(const std::string &key) const;

  // 获取指定请求头（不存在时返回空字符串）
  const std::string &operator[](HEADER header) const;
  const std::string &operator[](const std::string &key) const;

  // 获取请求体
  const std::string &body() const;

  // 解析HTTP原始字符串
  bool parse(const std::string &rawRequest);

 private:
  // 将字符串形式的请求方法转换成枚举类型
  static METHOD stringToMethod(const std::string &methodStr);

  // url解码
  static std::string urlDecode(const std::string &str);

  // 去除字符串首尾的空格
  static std::string trim(const std::string &str);

  // 解析请求行
  bool parseRequestLine(const std::string &line);

  // 解析请求参数
  void parseParameters(const std::string &paramStr);

  // 解析请求头
  void parseHeaders(const std::vector<std::string> &lines);

 private:
  METHOD method_;                                         // 请求方法
  std::string url_;                                       // 请求URL路径
  std::unordered_map<std::string, std::string> params_;   // URL查询参数
  std::string version_;                                   // 协议版本
  std::unordered_map<std::string, std::string> headers_;  // 请求头
  std::string body_;                                      // 请求体
  static const std::unordered_map<HEADER, std::string>
      header_map_;  // 请求头映射
};
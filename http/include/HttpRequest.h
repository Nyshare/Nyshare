#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class HttpRequest {
 public:
  // 请求方法的枚举常量
  enum Method {
    UNKNOWN = 0,
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE
  };

  HttpRequest();

  // 解析HTTP原始字符串
  bool parse(const std::string &rawRequest);

  // 成员变量的get方法
  Method getMethod() const;
  const std::string &methodToString() const;
  const std::string &getPath() const;
  const std::unordered_map<std::string, std::string> &getQueryParams() const;
  const std::string &getVersion() const;
  const std::unordered_map<std::string, std::string> &getHeaders() const;
  const std::string &getBody() const;

  // 请求参数与请求头单项get方法
  const std::string &getQueryParam(const std::string &key) const;
  const std::string &getHeader(const std::string &key) const;

  // 判断是否包含某个头部
  bool hasHeader(const std::string &key) const;

  // 将字符串形式的请求方法转换成枚举类型
  static Method parseMethod(const std::string &methodStr);
  // url解码
  static std::string urlDecode(const std::string &str);
  // 去除字符串首尾的空格
  static std::string trim(const std::string &str);

 private:
  // 解析请求行
  void parseRequestLine(const std::string &line);
  // 解析请求参数
  void parseParameters(const std::string &paramStr);
  // 解析请求头
  void parseHeaders(const std::vector<std::string> &lines);

 private:
  Method _method;                                             // 请求方法
  std::string _path;                                          // 请求URL路径
  std::unordered_map<std::string, std::string> _queryParams;  // URL查询参数
  std::string _version;                                       // 协议版本
  std::unordered_map<std::string, std::string> _headers;      // 请求头
  std::string _body;                                          // 请求体
};
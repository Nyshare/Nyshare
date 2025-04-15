#include "Emailer.h"

#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>

#include "Filer.h"
#include "Logger.h"

using json = nlohmann::json;

EmailSender &EmailSender::instance() {
  static EmailSender emailSender;
  return emailSender;
}

static size_t read_callback(char *ptr, size_t size, size_t nmemb,
                            void *userdata) {
  std::string *data = static_cast<std::string *>(userdata);
  if (data->empty()) return 0;  // 传输完成返回 0

  size_t to_copy = std::min(size * nmemb, data->size());
  memcpy(ptr, data->data(), to_copy);
  data->erase(0, to_copy);
  return to_copy;
}

bool EmailSender::sendVerificationCode(const std::string &emailAddress,
                                       const std::string &verificationCode) {
  CURL *curl = curl_easy_init();
  if (curl == nullptr) {
    return false;
  }

  curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());
  curl_easy_setopt(curl, CURLOPT_USERNAME, m_username.c_str());
  curl_easy_setopt(curl, CURLOPT_PASSWORD, m_password.c_str());
  curl_easy_setopt(curl, CURLOPT_MAIL_FROM, m_from.c_str());
  curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, "AUTH=LOGIN");

  struct curl_slist *headers = nullptr;
  headers = curl_slist_append(headers, "Expect:");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

  struct curl_slist *recipients =
      curl_slist_append(nullptr, emailAddress.c_str());
  curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

  std::string email =
      "From: <2389348329@qq.com>\r\n"
      "To: " +
      emailAddress +
      "\r\n"
      "Subject: 【您的验证码】请在 10 分钟内使用\r\n"
      "MIME-Version: 1.0\r\n"
      "Content-Type: text/html; charset=UTF-8\r\n"
      "\r\n"
      "<html>"
      "<body style='font-family: Arial, sans-serif; color: #333;'>"
      "<div style='max-width: 600px; margin: 0 auto; padding: 20px; border: "
      "1px solid #ddd; border-radius: 10px;'>"
      "    <h2 style='color: #007BFF;'>欢迎使用我们的服务！</h2>"
      "    <p>您好！</p>"
      "    <p>您正在尝试使用邮箱验证，以下是您的验证码：</p>"
      "    <p style='font-size: 24px; font-weight: bold; color: #007BFF;'>" +
      verificationCode +
      "</p>"
      "    <p>请在 <strong>10 分钟</strong> "
      "内输入验证码，否则验证码将失效。</p>"
      "    <p>如果这不是您的操作，请忽略此邮件。</p>"
      "    <br>"
      "    <p>感谢您的支持！</p>"
      "    <p><strong>【Nyshare】</strong></p>"
      "</div>"
      "</body>"
      "</html>"
      "\r\n";

  curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
  curl_easy_setopt(curl, CURLOPT_READDATA, &email);
  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(curl, CURLOPT_INFILESIZE, email.size());
  // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

  CURLcode res = curl_easy_perform(curl);

  curl_slist_free_all(headers);
  curl_slist_free_all(recipients);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    warn("[Emailer] Failed send verification code: %s",
         curl_easy_strerror(res));
    return false;
  }
  return true;
}

EmailSender::EmailSender() { loadConfig(); }

void EmailSender::loadConfig() {
  std::string config_string = Filer::readFile("../config/email.json");
  json config = json::parse(config_string);
  m_url = config.value("url", "");
  m_username = config.value("username", "");
  m_password = config.value("password", "");
  m_from = config.value("from", "");
}

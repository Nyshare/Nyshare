#pragma once
#include <curl/curl.h>

#include <string>

class EmailSender {
 public:
  static EmailSender &instance();

  bool sendVerificationCode(const std::string &emailAddress,
                            const std::string &verificationCode);

 private:
  EmailSender();
  EmailSender(const EmailSender &obj) = delete;
  const EmailSender &operator=(const EmailSender &obj) = delete;
  void loadConfig();

  std::string m_url;
  std::string m_username;
  std::string m_password;
  std::string m_from;
};
document.addEventListener("DOMContentLoaded", function () {
  const signupForm = document.getElementById("signupForm");
  const emailInput = document.getElementById("email");
  const verificationCodeInput = document.getElementById("verificationCode");
  const usernameInput = document.getElementById("username");
  const passwordInput = document.getElementById("password");
  const confirmPasswordInput = document.getElementById("confirmPassword");
  const sendCodeButton = document.getElementById("sendCodeButton");
  const signupButton = document.getElementById("signupButton");

  const emailError = document.getElementById("emailError");
  const captchaError = document.getElementById("captchaError");
  const usernameError = document.getElementById("usernameError");
  const passwordError = document.getElementById("passwordError");
  const confirmPasswordError = document.getElementById("confirmPasswordError");
  const errorMessage = document.getElementById("errorMessage");

  // 公共显示/隐藏错误提示函数
  function showError(el, msg) {
    el.textContent = msg;
    el.classList.add("show");
  }

  function hideError(el) {
    el.textContent = "";
    el.classList.remove("show");
  }

  // 验证函数
  function validateEmail() {
    const email = emailInput.value.trim();
    const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
    if (!emailRegex.test(email)) {
      showError(emailError, "请输入有效的邮箱地址");
      return false;
    }
    hideError(emailError);
    return true;
  }

  function validateVerificationCode() {
    const code = verificationCodeInput.value.trim();
    if (!/^\d{6}$/.test(code)) {
      showError(captchaError, "验证码必须是6位数字");
      return false;
    }
    hideError(captchaError);
    return true;
  }

  function validateUsername() {
    const username = usernameInput.value.trim();
    if (username.length === 0 || username.length > 16 || /\s/.test(username)) {
      showError(usernameError, "用户名必须在1到16个字符之间，且不能包含空格");
      return false;
    }
    hideError(usernameError);
    return true;
  }

  function validatePassword() {
    const password = passwordInput.value;
    const lengthValid = password.length >= 6 && password.length <= 32;
    const containsUpper = /[A-Z]/.test(password);
    const containsLower = /[a-z]/.test(password);
    const containsDigit = /\d/.test(password);
    const containsSpecial = /[!@#$%^&*(),.?":{}|<>]/.test(password);

    if (lengthValid && containsUpper && containsLower && containsDigit && containsSpecial) {
      hideError(passwordError);
      return true;
    }

    showError(passwordError, "密码必须6-32位，包含大小写字母、数字和特殊字符");
    return false;
  }

  function validateConfirmPassword() {
    if (confirmPasswordInput.value !== passwordInput.value) {
      showError(confirmPasswordError, "两次密码输入不一致");
      return false;
    }
    hideError(confirmPasswordError);
    return true;
  }

  // 绑定验证事件
  emailInput.addEventListener("blur", validateEmail);
  verificationCodeInput.addEventListener("blur", validateVerificationCode);
  usernameInput.addEventListener("blur", validateUsername);
  passwordInput.addEventListener("blur", validatePassword);
  confirmPasswordInput.addEventListener("blur", validateConfirmPassword);

  // 验证码倒计时
  let countdown = 60;
  let timer;

  sendCodeButton.addEventListener("click", function () {
    if (!validateEmail()) return;

    sendCodeButton.disabled = true;
    sendCodeButton.textContent = `${countdown} 秒后重试`;

    timer = setInterval(() => {
      countdown--;
      sendCodeButton.textContent = `${countdown} 秒后重试`;
      if (countdown <= 0) {
        clearInterval(timer);
        sendCodeButton.disabled = false;
        sendCodeButton.textContent = "发送验证码";
        countdown = 60;
      }
    }, 1000);

    fetch("/api/send_verification_code", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ email: emailInput.value.trim() })
    })
      .then(res => res.json())
      .then(data => {
        if (!data.success) {
          clearInterval(timer);
          sendCodeButton.disabled = false;
          sendCodeButton.textContent = "发送验证码";
          countdown = 60;
          showError(captchaError, data.message || "发送验证码失败");
        }
      })
      .catch(() => {
        clearInterval(timer);
        sendCodeButton.disabled = false;
        sendCodeButton.textContent = "发送验证码";
        countdown = 60;
        showError(captchaError, "网络错误，请稍后重试");
      });
  });

  // 提交注册
  signupForm.addEventListener("submit", function (e) {
    e.preventDefault();
    if (
      !validateEmail() ||
      !validateVerificationCode() ||
      !validateUsername() ||
      !validatePassword() ||
      !validateConfirmPassword()
    ) {
      return;
    }

    signupButton.disabled = true;

    fetch("/api/signup", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        email: emailInput.value.trim(),
        verification_code: verificationCodeInput.value.trim(),
        username: usernameInput.value.trim(),
        password: passwordInput.value
      })
    })
      .then(res => res.json())
      .then(data => {
        if (data.success) {
          window.location.href = "/login.html";
        } else {
          showError(errorMessage, data.message || "注册失败");
          signupButton.disabled = false;
        }
      })
      .catch(() => {
        showError(errorMessage, "网络错误，请稍后再试");
        signupButton.disabled = false;
      });
  });
});

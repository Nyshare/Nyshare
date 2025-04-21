document.addEventListener("DOMContentLoaded", function () {
  const loginForm = document.getElementById("loginForm");
  const loginButton = document.getElementById("loginButton");
  const errorMessage = document.getElementById("errorMessage");

  loginForm.addEventListener("submit", async function (event) {
    event.preventDefault(); // 阻止默认提交行为

    const username = document.getElementById("username").value.trim();
    const password = document.getElementById("password").value.trim();

    // 清空错误信息
    errorMessage.textContent = "";
    errorMessage.style.display = "none";

    // 表单校验
    if (!username || !password) {
      showError("用户名和密码不能为空！");
      return;
    }

    // 禁用按钮，防止重复点击
    loginButton.disabled = true;
    loginButton.textContent = "登录中...";

    try {
      // 发送请求
      const response = await fetch("/api/login", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ username, password })
      });

      const data = await response.json();

      if (data.success) {
        // ✅ 存储用户信息到 localStorage
        localStorage.setItem("token", data.token);

        // ✅ 登录成功，跳转到主页
        window.location.href = "/";
      } else {
        showError(data.message || "登录失败，请重试！");
      }
    } catch (error) {
      console.error("请求错误:", error);
      showError("服务器异常，请稍后再试！");
    } finally {
      // 解除按钮禁用
      loginButton.disabled = false;
      loginButton.textContent = "登录";
    }
  });

  function showError(message) {
    errorMessage.textContent = message;
    errorMessage.style.display = "block";
  }
});

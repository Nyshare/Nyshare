document.getElementById('uploadForm').addEventListener('submit', async function (event) {
  event.preventDefault(); // 阻止默认表单提交

  const submitButton = event.target.querySelector('button[type="submit"]'); // 获取提交按钮
  submitButton.disabled = true; // 提交后禁用按钮，防止重复提交

  // 获取表单数据
  const expireAt = document.getElementById('expire_at').value;
  const text = document.getElementById('text').value;

  // 校验输入
  if (!expireAt || !text) {
    alert('请填写所有字段');
    submitButton.disabled = false; // 如果表单无效，重新启用按钮
    return;
  }

  // 构建请求体
  const requestData = {
    expire_at: expireAt,
    text: text
  };

  try {
    // 发送 POST 请求，携带 token
    const response = await fetch('/api/upload_post', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${localStorage.getItem('token')}`
      },
      body: JSON.stringify(requestData)
    });

    const data = await response.json(); // 解析 JSON 响应

    // 根据响应结果显示相应的提示
    const resultDiv = document.getElementById('result');
    if (data.success) {
      resultDiv.style.color = 'green';
      resultDiv.textContent = data.message || '作品上传成功！';
    } else {
      resultDiv.style.color = 'red';
      resultDiv.textContent = data.message || '上传失败！';
      submitButton.disabled = false; // 失败时重新启用按钮，用户可以再次提交
    }
  } catch (error) {
    console.error('请求失败:', error);
    const resultDiv = document.getElementById('result');
    resultDiv.style.color = 'red';
    resultDiv.textContent = '请求失败，请稍后重试。';
    submitButton.disabled = false; // 请求失败时重新启用按钮
  }
});

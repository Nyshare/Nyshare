document.addEventListener('DOMContentLoaded', () => {
  // 假设从服务器获取作品数据的 API
  const apiUrl = '/api/get_posts'; // 请替换为你的实际 API 地址

  // 获取作品数据并动态生成 HTML
  fetch(apiUrl)
    .then(response => response.json()) // 解析 JSON 数据
    .then(data => {
      // 渲染作品内容到页面
      const postsContainer = document.getElementById('posts-container');

      data.posts.forEach(post => {
        // 创建作品卡片
        const postCard = document.createElement('div');
        postCard.classList.add('post-card');

        // 作品内容
        const postContent = document.createElement('div');
        postContent.classList.add('post-content');
        // 初始内容提示
        postContent.innerHTML = `<p class="post-text">加载中...</p>`;

        // 将内容添加到卡片
        postCard.appendChild(postContent);

        // 加载文本内容（从 content 字段的 URL 读取）
        fetch(post.text_url)
          .then(res => res.text())
          .then(text => {
            const maxLength = 100; // 最多显示100个字符

            const fullText = text;
            const shortText = text.length > maxLength ? text.substring(0, maxLength) + '...' : text;

            const isLong = text.length > maxLength;

            // 设置初始内容
            postContent.innerHTML = `
      <pre class="post-text">${shortText}</pre>
      ${isLong ? '<button class="toggle-btn">展开</button>' : ''}
    `;
            if (isLong) {
              const btn = postContent.querySelector('.toggle-btn');
              let expanded = false;

              btn.addEventListener('click', () => {
                expanded = !expanded;
                postContent.querySelector('.post-text').textContent = expanded ? fullText : shortText;
                btn.textContent = expanded ? '收起' : '展开';
              });
            }

            // 将作品卡片插入到页面中
            postsContainer.appendChild(postCard);
          });

      })
        .catch(error => {
          console.error('请求作品数据失败:', error);
        });
    });
});

document.getElementById('post-btn').addEventListener('click', () => {
  const token = localStorage.getItem('token');
  if (!token) {
    alert('未登录');
    window.location.href = '/login.html';
    return;
  }

  fetch('/upload.html', {
    headers: {
      'Authorization': `Bearer ${token}`
    }
  })
    .then(res => {
      if (!res.ok) throw new Error('加载失败');
      return res.text();
    })
    .then(html => {
      // 在当前页面替换成投稿页
      document.open();
      document.write(html);
      document.close();
    })
    .catch(err => {
      alert('无法加载投稿页');
      console.error(err);
    });
});

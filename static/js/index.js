document.addEventListener('DOMContentLoaded', () => {
  // 假设从服务器获取作品数据的 API
  const apiUrl = '/api/posts'; // 请替换为你的实际 API 地址

  // 获取作品数据并动态生成 HTML
  fetch(apiUrl)
    .then(response => response.json()) // 解析 JSON 数据
    .then(data => {
      // 渲染作品内容到页面
      const postsContainer = document.getElementById('posts-container');
      data.forEach(post => {
        // 创建作品卡片
        const postCard = document.createElement('div');
        postCard.classList.add('post-card');

        // 作品内容
        const postContent = document.createElement('div');
        postContent.classList.add('post-content');
        postContent.innerHTML = `<p class="post-text">${post.content}</p>`;

        // 作品操作按钮
        const postActions = document.createElement('div');
        postActions.classList.add('post-actions');
        postActions.innerHTML = `
          <button class="btn like-btn">👍 点赞</button>
          <button class="btn comment-btn">💬 评论</button>
          <button class="btn favorite-btn">⭐ 收藏</button>
          <button class="btn share-btn">🔗 分享</button>
        `;

        // 将内容添加到卡片
        postCard.appendChild(postContent);
        postCard.appendChild(postActions);

        // 将作品卡片插入到页面中
        postsContainer.appendChild(postCard);
      });

      // 为每个按钮绑定点击事件
      document.querySelectorAll('.like-btn').forEach(button => {
        button.addEventListener('click', () => {
          alert('你点了个赞 👍');
        });
      });

      document.querySelectorAll('.comment-btn').forEach(button => {
        button.addEventListener('click', () => {
          alert('跳转到评论区 💬');
        });
      });

      document.querySelectorAll('.favorite-btn').forEach(button => {
        button.addEventListener('click', () => {
          alert('已收藏该作品 ⭐');
        });
      });

      document.querySelectorAll('.share-btn').forEach(button => {
        button.addEventListener('click', () => {
          alert('分享链接已复制 🔗');
        });
      });

    })
    .catch(error => {
      console.error('请求作品数据失败:', error);
    });
});

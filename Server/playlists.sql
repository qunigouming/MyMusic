CREATE TABLE playlists (
  id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY COMMENT '歌单ID',
  user_id INT UNSIGNED NOT NULL COMMENT '用户ID',
  name VARCHAR(255) NOT NULL COMMENT '歌单名称',
  description TEXT COMMENT '歌单描述',
  cover_url VARCHAR(512) COMMENT '歌单封面URL',
  is_default TINYINT(1) NOT NULL DEFAULT 0 COMMENT '是否为默认歌单（1是，0否）',
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  FOREIGN KEY (user_id) REFERENCES user(id) ON DELETE CASCADE,
  INDEX idx_user_playlists (user_id),
  UNIQUE KEY uniq_user_default (user_id, is_default) COMMENT '确保每个用户只有一个默认歌单'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='用户歌单表';

CREATE TABLE albums (
  id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY COMMENT '专辑ID',
  title VARCHAR(255) NOT NULL COMMENT '专辑名称',
  release_date DATE COMMENT '发行日期',
  cover_url VARCHAR(512) COMMENT '专辑封面URL',
  description TEXT COMMENT '专辑描述',
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  
  FOREIGN KEY (artist_id) REFERENCES artists(id) ON DELETE SET NULL,
  INDEX idx_album_title (title),
  INDEX idx_release_date (release_date)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='专辑信息表';

CREATE TABLE playlist_songs (
  playlist_id INT UNSIGNED NOT NULL COMMENT '歌单ID',
  song_id INT UNSIGNED NOT NULL COMMENT '歌曲ID',
  position INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '歌曲在歌单中的顺序',
  added_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '添加时间',
  PRIMARY KEY (playlist_id, song_id),
  FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE,
  FOREIGN KEY (song_id) REFERENCES songs(id) ON DELETE CASCADE,
  INDEX idx_position (position),
  INDEX idx_playlist_position (playlist_id, position)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='歌单歌曲关联表';

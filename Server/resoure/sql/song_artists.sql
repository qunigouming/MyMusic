CREATE TABLE song_artists (
  song_id INT UNSIGNED NOT NULL COMMENT '歌曲ID',
  artist_id INT UNSIGNED NOT NULL COMMENT '歌手ID',
  PRIMARY KEY (song_id, artist_id),
  FOREIGN KEY (song_id) REFERENCES songs(id) ON DELETE CASCADE,
  FOREIGN KEY (artist_id) REFERENCES artists(id) ON DELETE CASCADE,
  INDEX idx_artist (artist_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='歌曲与歌手关联表';

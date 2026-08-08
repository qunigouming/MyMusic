CREATE TABLE album_artists (
  album_id INT UNSIGNED NOT NULL COMMENT '专辑ID',
  artist_id INT UNSIGNED NOT NULL COMMENT '歌手ID',
  PRIMARY KEY (album_id, artist_id),
  FOREIGN KEY (album_id) REFERENCES albums(id) ON DELETE CASCADE,
  FOREIGN KEY (artist_id) REFERENCES artists(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='专辑与歌手关联表';

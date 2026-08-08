create table file_map (
	id int unsigned auto_increment primary key,
	storage_path varchar(512) not null,
	mime_type varchar(100),
	created_by int unsigned,
	created_at timestamp default current_timestamp,
	index idx_created_by (created_by)
);

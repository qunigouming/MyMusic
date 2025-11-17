create table files (
	id int unsigned auto_increment primary key,
	filename varchar(255) not null,
	file_path varchar(512) not null,
	mime_type varchar(100),
	size int unsigned,
	created_by int unsigned,
	created_at timestamp default current_timestamp,
	index idx_created_by (created_by)
);

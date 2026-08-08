create table user (
	id int unsigned AUTO_INCREMENT primary key not null comment '主键ID',
	name varchar(255) unique key not null comment '用户名',
	email varchar(255) unique key not null comment '邮箱',
	password_salt varchar(255) not null comment '密码盐值',
	password_hash varchar(255) not null comment '密码哈希值',
	icon varchar(255) not null default ':/source/image/default_user_head.png' comment '头像',
	sex tinyint not null default 0 comment '性别(0-未知, 1-男, 2-女)',
	created_at timestamp default current_timestamp comment '创建时间',
	updated_at timestamp default current_timestamp on update current_timestamp comment '更新时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC comment='用户表';

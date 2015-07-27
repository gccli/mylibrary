use repl;
drop table users;
drop table users_2;
CREATE TABLE users (
  id       int(10) unsigned NOT NULL AUTO_INCREMENT,
  username varchar(128) NOT NULL,
  domain   varchar(128) NOT NULL,
  password varchar(64) NOT NULL,
  home     varchar(256) NOT NULL,
  uid      int(11) NOT NULL DEFAULT '1000',
  gid      int(11) NOT NULL DEFAULT '1000',
  active   char(1) NOT NULL DEFAULT 'Y',
  regtime  timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY(id),
  UNIQUE KEY(username),
  KEY(domain)
);

CREATE TABLE users_2 (
  id       int(10) unsigned NOT NULL AUTO_INCREMENT,
  username varchar(128) NOT NULL,
  domain   varchar(128) NOT NULL,
  password varchar(64) NOT NULL,
  home     varchar(256) NOT NULL,
  uid      int(11) NOT NULL DEFAULT '1000',
  gid      int(11) NOT NULL DEFAULT '1000',
  active   char(1) NOT NULL DEFAULT 'Y',
  regtime  timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY(id,username)
);
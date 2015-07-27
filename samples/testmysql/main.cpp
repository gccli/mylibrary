#include "mysqlext/myapp.h"
#include <stdio.h>
#include <errno.h>
#include <string>
#include <iostream>

int test_mysql(int argc, char* argv[])
{
	if (argc < 2) 
	{
		printf ("Usage: \n\t./a.out filename [...]\n");
		return 1;
	}

	const char* host = "172.16.0.10"; // Mysql数据库服务器所在主机，使用默认端口3306，亦可指定端口，如"localhost:3000"或"10.10.168.128:3000"
	const char* user = "root";      // Mysql用户
	std::string pass;               // Mysql用户密码
	const char* db   = "test";      // Mysql数据库名

	printf ("输入本地MySQL数据库的root密码: ");
	std::getline(std::cin, pass);

	MySQL::Connection conn;
	if (!conn.connect (host, user, pass.c_str(), db))
	{
		return 1;
	}

	// 创建一个具有BLOG字段的数据库表
	char drop_table_sql[]   = "DROP TABLE IF EXISTS T_TEST";
	char create_table_sql[] = "CREATE TABLE T_TEST\
		(\
			ID			SERIAL,\
			NAME		VARCHAR(100),\
			DATA_LEN	DECIMAL(10,0),\
			DATA		MEDIUMBLOB,\
			PRIMARY KEY  (ID)\
		)";
	
	if (!conn.execute (drop_table_sql))
		return 1;

	if (!conn.execute (create_table_sql))
		return 1;

	// [1] 写文件到数据库表 T_TEST
	printf ("[1] 写文件到数据库表 T_TEST\n");
	for (int i=1; i<argc; ++i)
	{
		// 读取将要写入数据库的文件
		char filename[256] = "\0";
		strcpy (filename, argv[i]);
		FILE* fp = fopen (filename, "rb");
		if (fp == NULL)
		{
			fprintf (stderr, "打开文件失败:%s", strerror (errno));
			return 1;
		}

		struct mysql_iovec v;
		fseek(fp, 0, SEEK_SET);
		fseek(fp, 0, SEEK_END);
		v.iov_len = ftell (fp);
		unsigned char* pBuf = new unsigned char [v.iov_len];
		memset (pBuf, 0, v.iov_len);

		fseek(fp, 0, SEEK_SET);
		fread (pBuf, 1, v.iov_len, fp);
		fclose (fp);

		// 获取文件名(basename)
		const char delim[] = "/\\"; // 文件分隔符
		char* basename = strrchr (filename, delim[0]);
		if (!basename)
		{
			basename = strrchr (filename, delim[1]);
		}
		if (basename){
			basename++;
		}
		else {
			basename = filename;
		}

		// 写BLOG
		char sql[1024] = "\0";
		sprintf (sql, "INSERT INTO T_TEST (NAME,DATA_LEN,DATA) VALUES ('%s',%d,?)", 
			basename, v.iov_len);

		v.iov_base = pBuf;
		if (conn.query()->blob_write (sql, &v, 1))
		{
			printf ("写入文件\"%s\"到数据表T_TEST成功，共%d字节.\n", basename, v.iov_len);
		}
		else 
		{
			printf ("写入文件失败:%s\n", conn.error().c_str());
		}

		delete [] pBuf;
	}

	// [2] 读取T_TEST表，并将BOLG字段中的数据写到文件
	printf ("\n[2] 读取T_TEST表，并将BOLG字段中的数据写到文件\n");
	char sql[] = "SELECT ID,NAME,DATA_LEN,DATA FROM T_TEST";
	if (!conn.query()->execute (sql)){
		printf ("执行数据库语句\"%s\"失败:%s.", sql, conn.error().c_str());	
		return 1;
	}

	MySQL::Row *r;
	while (r = conn.query()->fetch())
	{
		int i = 0;
		int id = (*r)[i++];
		char* name = (*r)[i++];
		int datalen = (*r)[i++];
		char* pBuf = (*r)[i++];
		
		char TmpFile[256] = "\0";
#ifdef WIN32
		GetTempPath(MAX_PATH, TmpFile);
#else
		strcpy (TmpFile, "/tmp/");
#endif
		strcat (TmpFile, name);
		FILE* fp = fopen (TmpFile, "wb");
		if (fp)
		{
			if (fwrite (pBuf, 1, datalen, fp) == datalen){
				printf ("读出BLOG写入文件\"%s\"（%d字节）.\n", TmpFile, datalen);
			}
			fclose (fp);
		}
	}

	return 0;
}



int test_mysql1(int argc, char* argv[])
{
	const char* host = "172.27.237.60"; // Mysql数据库服务器所在主机，使用默认端口3306，亦可指定端口，如"localhost:3000"或"10.10.168.128:3000"
	const char* user = "lijing";      // Mysql用户
	std::string pass = "cc";               // Mysql用户密码
	const char* db   = "repl";      // Mysql数据库名

	std::getline(std::cin, pass);

	MySQL::Connection conn;
	if (!conn.connect (host, user, pass.c_str(), db))
	{
		return 1;
	}

	// 创建一个具有BLOG字段的数据库表
	char drop_table_sql[]   = "DROP TABLE IF EXISTS T_TEST";
	char create_table_sql[] = "CREATE TABLE T_TEST\
		(\
			ID			SERIAL,\
			NAME		VARCHAR(100),\
			DATA_LEN	DECIMAL(10,0),\
			DATA		MEDIUMBLOB,\
			PRIMARY KEY  (ID)\
		)";
	
	if (!conn.execute (drop_table_sql))
		return 1;

	if (!conn.execute (create_table_sql))
		return 1;
	
	if (!conn.execute("insert into T_TEST(NAME, DATA_LEN) values('cc', 0)")) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.execute("set autocommit=0")) { // conn.query()->autocommit(false)
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.execute("insert into T_TEST(NAME, DATA_LEN) values('12', 0)")) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.query()->rollback()) {// ROLLBACK
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.execute("insert into T_TEST(NAME, DATA_LEN) values('34', 0)")) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.query()->commit()) { // COMMIT
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.query()->autocommit()) { // set autocommit=1
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.execute("begin")) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.execute("insert into T_TEST(NAME, DATA_LEN) values('56', 2)")) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.execute("insert into T_TEST(NAME, DATA_LEN) values('78', 2)")) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.query()->commit()) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.execute("start transaction")) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.execute("insert into T_TEST(NAME, DATA_LEN) values('55', 9)")) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.execute("insert into T_TEST(NAME, DATA_LEN) values('66', 9)")) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.query()->rollback()) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	if (!conn.execute("insert into T_TEST(NAME, DATA_LEN) values('77', 9)")) {
		printf ("error:%s\n", conn.error().c_str());
		return 1;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	return test_mysql1(argc, argv);
}

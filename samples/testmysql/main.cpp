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

	const char* host = "172.16.0.10"; // Mysql���ݿ����������������ʹ��Ĭ�϶˿�3306�����ָ���˿ڣ���"localhost:3000"��"10.10.168.128:3000"
	const char* user = "root";      // Mysql�û�
	std::string pass;               // Mysql�û�����
	const char* db   = "test";      // Mysql���ݿ���

	printf ("���뱾��MySQL���ݿ��root����: ");
	std::getline(std::cin, pass);

	MySQL::Connection conn;
	if (!conn.connect (host, user, pass.c_str(), db))
	{
		return 1;
	}

	// ����һ������BLOG�ֶε����ݿ��
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

	// [1] д�ļ������ݿ�� T_TEST
	printf ("[1] д�ļ������ݿ�� T_TEST\n");
	for (int i=1; i<argc; ++i)
	{
		// ��ȡ��Ҫд�����ݿ���ļ�
		char filename[256] = "\0";
		strcpy (filename, argv[i]);
		FILE* fp = fopen (filename, "rb");
		if (fp == NULL)
		{
			fprintf (stderr, "���ļ�ʧ��:%s", strerror (errno));
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

		// ��ȡ�ļ���(basename)
		const char delim[] = "/\\"; // �ļ��ָ���
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

		// дBLOG
		char sql[1024] = "\0";
		sprintf (sql, "INSERT INTO T_TEST (NAME,DATA_LEN,DATA) VALUES ('%s',%d,?)", 
			basename, v.iov_len);

		v.iov_base = pBuf;
		if (conn.query()->blob_write (sql, &v, 1))
		{
			printf ("д���ļ�\"%s\"�����ݱ�T_TEST�ɹ�����%d�ֽ�.\n", basename, v.iov_len);
		}
		else 
		{
			printf ("д���ļ�ʧ��:%s\n", conn.error().c_str());
		}

		delete [] pBuf;
	}

	// [2] ��ȡT_TEST������BOLG�ֶ��е�����д���ļ�
	printf ("\n[2] ��ȡT_TEST������BOLG�ֶ��е�����д���ļ�\n");
	char sql[] = "SELECT ID,NAME,DATA_LEN,DATA FROM T_TEST";
	if (!conn.query()->execute (sql)){
		printf ("ִ�����ݿ����\"%s\"ʧ��:%s.", sql, conn.error().c_str());	
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
				printf ("����BLOGд���ļ�\"%s\"��%d�ֽڣ�.\n", TmpFile, datalen);
			}
			fclose (fp);
		}
	}

	return 0;
}



int test_mysql1(int argc, char* argv[])
{
	const char* host = "172.27.237.60"; // Mysql���ݿ����������������ʹ��Ĭ�϶˿�3306�����ָ���˿ڣ���"localhost:3000"��"10.10.168.128:3000"
	const char* user = "lijing";      // Mysql�û�
	std::string pass = "cc";               // Mysql�û�����
	const char* db   = "repl";      // Mysql���ݿ���

	std::getline(std::cin, pass);

	MySQL::Connection conn;
	if (!conn.connect (host, user, pass.c_str(), db))
	{
		return 1;
	}

	// ����һ������BLOG�ֶε����ݿ��
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

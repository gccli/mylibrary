#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/syscall.h>

#include <pthread.h>
#include <mysql/mysql.h>

#include <assert.h>
#include <getopt.h>

#include "utiltime.h"

#define THREAD_COUNT 4
#define gettid() syscall(__NR_gettid)

const char *host;
const char *user        = "root";
const char *pass        = NULL;
const char *dbname      = "repl";
const char *unix_socket = NULL;

MYSQL db;

int verbose = 0;
static pthread_mutex_t mutex;
int MYSQL_CONNECT()
{
	pthread_mutex_init(&mutex, NULL);
	int port = 3306;
	int timeout = 15;
	my_bool reconnect = 1;

	mysql_init(&db);
	mysql_options(&db, MYSQL_OPT_RECONNECT, &reconnect);
    mysql_options(&db, MYSQL_OPT_READ_TIMEOUT, (char *)&timeout);
    mysql_options(&db, MYSQL_OPT_WRITE_TIMEOUT, (char *)&timeout);

	MYSQL *pHandle = mysql_real_connect(&db,host,user,pass,dbname,port,unix_socket,CLIENT_MULTI_STATEMENTS);
	if (pHandle == NULL) {
		printf("connect failure: %s\n", mysql_error(&db));
		return 1;
	}
	mysql_options(pHandle, MYSQL_SET_CHARSET_NAME, "utf8");
	mysql_set_character_set(pHandle,"utf8");
	mysql_autocommit(pHandle, 0);

	return 0;
}


int MYSQL_QUERY(const char *sql, int len)
{
	return mysql_real_query(&db, sql, len);
}

int MYSQL_SELECT(const char *sql)
{
	int iret = 0, retry = 1;
	pthread_mutex_lock(&mutex);

retry:
	if ((iret = mysql_query(&db, sql)) != 0 && retry) {
		printf("query [%s] failure: %s\n", sql, mysql_error(&db));
		retry = 0;
		sleep(1);
		mysql_ping(&db);
		goto retry;
	}
	if (iret != 0)
	{
		pthread_mutex_unlock(&mutex);
		return 1;
	}
	MYSQL_RES *result = mysql_store_result(&db);
	if (result == NULL) {
		printf("query [%s] failure: %s\n", sql, mysql_error(&db));
		pthread_mutex_unlock(&mutex);
		return 1;
	}
	pthread_mutex_unlock(&mutex);


	MYSQL_ROW row;
	int i, count = mysql_num_rows(result);
	printf("THREAD[%ld] %d rows in sets\n", gettid(), count);
	unsigned int num_fields = mysql_num_fields(result);
	while ((row = mysql_fetch_row(result)))
	{
		unsigned long *lengths = mysql_fetch_lengths(result);
		for(i = 0; i < num_fields; i++)
		{
			if(verbose)printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL");
		}
		if(verbose)printf("\n");
	}
	mysql_free_result(result);
	
	return 0;
}


void *threadfunc1(void *param)
{
	int  i=0;
	char sql[1024];
	for (;i<300; ++i) 
	{
		int id = random() % 100;
		sprintf(sql, "select * from users where id = %d", id);
		assert(MYSQL_SELECT(sql) == 0);
		usleep(1);
	}

	return NULL;
}

void *threadfunc2(void *param)
{
	int  i=0;
	char sql[1024];
	for (;i<3000; ++i) 
	{
		int id = random() % 100;
		sprintf(sql, "select * from users where id < %d", id);
		assert(MYSQL_SELECT(sql) == 0);
		usleep(1);
	}

	return NULL;
}

void PARSE_CLI(int argc, char *argv[])
{
	static struct option long_options[] = {
		{0, 0, 0, 0}
	};
	int index = 0;
	const char* optlist = "h:u:p:d:s:";
	while (1){
		int c = getopt_long(argc, argv, optlist, long_options, &index);
		if (c == EOF) break;
		switch (c) {
		case 'h':
			host = strdup(optarg);
			break;
		case 'u':
			user = strdup(optarg);
			break;
		case 'p':
			pass = strdup(optarg);
			break;
		case 'd':
			dbname = strdup(optarg);
			break;
		case 's':
			unix_socket = strdup(optarg);
			break;			
		case 0:
			break;
		default:
			printf("usage: %s [-h host] [-u user] [-p passwd] [-d db] [-s sock]\n", argv[0]);
			exit(0);
		}
	}
}

int TEST_MULTITHREAD()
{
	int i;
	pthread_t th[THREAD_COUNT+1];
	for (i=0; i<THREAD_COUNT; i+=2) 
	{
		pthread_create(&th[i], NULL, threadfunc1, NULL);
		pthread_create(&th[i+1], NULL, threadfunc2, NULL);
	}

	for (i=0; i<THREAD_COUNT; ++i) 
	{
		pthread_join(th[i], NULL);
	}

	return 0;
}



int TEST_INTER_JOIN()
{
	double tstart;
	const char *sql = "use world";
	MYSQL_QUERY(sql, strlen(sql));
	// Warm up cache
	MYSQL_SELECT("SELECT C1.Name,C2.Name from City C1, Country C2 where C1.CountryCode=C2.Code and C2.Name='China';");

	// 
	printf("\n1. No Inner Join\n");
	tstart = timing_start();
	MYSQL_SELECT("SELECT C1.Name,C2.Name from City C1, Country C2 where C1.CountryCode=C2.Code and C2.Name='China';");
	printf("time cost %lf second\n", timing_cost(tstart));

	printf("\n2. Inner Join\n");
	tstart = timing_start();
	MYSQL_SELECT("SELECT C1.Name,C2.Name from City C1 INNER JOIN Country C2 ON C1.CountryCode=C2.Code WHERE C2.Name='China';");
	printf("time cost %lf second\n", timing_cost(tstart));

	printf("\n3. No Inner Join\n");
	tstart = timing_start();
	MYSQL_SELECT("SELECT C1.Name,C2.Name from City C1, Country C2 where C1.CountryCode=C2.Code and C2.Name='China';");
	printf("time cost %lf second\n", timing_cost(tstart));

	return 0;
}

int main(int argc, char *argv[])
{
	PARSE_CLI(argc, argv);
	MYSQL_CONNECT();

	TEST_INTER_JOIN();
	return 0;
}


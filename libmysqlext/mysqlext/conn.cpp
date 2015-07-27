#include "conn.h"

using namespace MySQL;
Connection::Connection(void) 
{
    _query = NULL;
}

Connection::~Connection(void) 
{
    disconnect();
}

bool Connection::connect(const char* host, const char* user, const char* passwd, const char* db)
{
	const char addr_delim[] = "/:";

	char	ip[32] = "\0";
	int		port = 3306;
	strncpy (ip, host, sizeof (ip));
	char* p = strpbrk (ip, addr_delim);
	if (p){
		*p = 0;
		port = atoi (p+1);
	}

	MySQL::Query* q = query();
	if (!q) {
		return false;
	}
	MYSQL* h = query()->handle();
	if (!h){
		return false;
	}

	if (mysql_real_connect (h, ip, user, passwd, db, port, NULL, 0) == NULL)
	{
		fprintf (stderr, "Connect to MySQL Failed: %s\n", error().c_str());
		return false;
	}

	unsigned long ver = mysql_get_server_version (query()->handle());
	char strver[32] = "\n";	
	sprintf (strver, "%ld.%ld.%ld", ver/10000, (ver/100)%100, ver%100);
	m_version = strver;
	
	printf ("Connect to MySQL server successfully, version:%s.\n\n", m_version.c_str());
	fflush (stdout);

    return true;
}


/*
 * Here we deallocate things in reverse order, since they have
 * dependencies on each other.
 */

bool Connection::disconnect(void) 
{
	if (_query) {
		delete _query;
		_query = NULL;
	}
	m_version = "";

	return true;
}

std::string Connection::error(void)
{
	return query()->error();
}

const std::string &Connection::version(void)
{
	return m_version;
}

/*
 * This is the prime place to parallelize the MySQL library -- here
 * we're just caching one Query object, but we could potentially cache
 * many more, and this would be the once place to change the logic and
 * dole them out, perhaps limiting the total number, how many are left
 * around, etc.
 */

MySQL::Query *Connection::query(void)
{
	if (_query)
		return _query;

	_query = new Query();

	if (!_query->init()) {
		fprintf (stderr, "Failed to initialize query object.\n");
		delete _query;
		_query = NULL;
	}

	return _query;
}

bool Connection::execute(const char *s) 
{
	MySQL::Query *q = query();
	return q ? q->execute(s) : false;
}


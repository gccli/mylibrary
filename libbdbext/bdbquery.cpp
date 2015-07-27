#include "bdbquery.h"

using namespace bdbext;

Query::Query(const char* dbfile)
	:m_dbenv(NULL)
	,m_dbp(NULL)
	,m_dbc(NULL)
	,m_txnid(NULL)
	,m_dbfile(NULL)
{
	if (dbfile != NULL) {
		int length = strlen(dbfile);
		m_dbfile = new char[length+1];
		strcpy (m_dbfile, dbfile);
		m_dbfile[length] = 0;
	}
}

Query::~Query()
{
	
}

int Query::init(bool isdbenv)
{
	int ret;
	unsigned int flags = DB_CREATE | DB_INIT_MPOOL; // in-memory cache

	if (isdbenv) {
		// environment
		if ((ret = db_env_create (&m_dbenv, 0)) != 0) {
			printf ("db_env_create: %s\n", db_strerror(ret));
			return 1;
		}

		if ((ret = m_dbenv->open (m_dbenv, "dbenv", flags, 0)) != 0) {
			printf ("dbenv->open: %s\n", db_strerror(ret));
			return 1;
		}
	}
	if ((ret = db_create (&m_dbp, m_dbenv, 0)) != 0){
		printf ("db_create: %s\n", db_strerror(ret));
		return 1;
	}	

	return 0;
}

int Query::open(const char* dbname, unsigned int flags)
{
	int ret;
	if ((ret = m_dbp->open (m_dbp, m_txnid, m_dbfile, dbname, DB_BTREE,	flags, 0600)) != 0) {
		printf ("dbp->open: %s\n", db_strerror(ret));
		return 1;
	}

	return 0;
}

int Query::opencursor(unsigned int flags)
{
	int ret;
	if ((ret = m_dbp->cursor(m_dbp, NULL, &m_dbc, flags)) != 0) {
		printf ("dbp->cursor: %s\n", db_strerror(ret));
		return 1;
	}
	
	return 0;
}


int Query::read(void* key, int klen, void* value, int* vlen, unsigned int flags)
{
	int ret;

	DBT _key, _value;
	memset (&_key,  0, sizeof(DBT));
	memset (&_value, 0, sizeof(DBT));

	_key.data = key;
	_key.size = klen;

	if ((ret = m_dbp->get(m_dbp, m_txnid, &_key, &_value, flags)) != 0) {
		printf ("dbp->get: %s\n", db_strerror(ret));
		return 1;
	}

	*vlen = _value.size;
	memcpy (value, _value.data, _value.size);

	return 0;
}

int Query::write(DBT* key, DBT* value, unsigned int flags)
{
	int ret;
	if ((ret = m_dbp->put(m_dbp, m_txnid, key, value, flags)) != 0) {
		m_dbp->err (m_dbp, ret, "dbp->put");
		return 1;
	}
  
	return 0;
}

int Query::readcur(DBT* key, DBT* value, unsigned int flags)
{
	return m_dbc->get(m_dbc, key, value, flags);
}


int Query::sync(unsigned int flags)
{
	return m_dbp->sync(m_dbp, flags);
}

int Query::close(unsigned int flags)
{
	return m_dbp->close(m_dbp, flags);
}


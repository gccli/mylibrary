#ifndef BDB_QUERY_H_
#define BDB_QUERY_H_

#include "bdbconfig.h"

namespace bdbext {

class Query 
{
public:
	Query(const char* dbfile);
	virtual ~Query();

	int init(bool isdbenv);
	int open(const char* dbname, unsigned int flags);
	int opencursor(unsigned int flags);
	int read(void* key, int klen, void* value, int* vlen, unsigned int flags);
	int readcur(DBT* key, DBT* value, unsigned int flags);
	int write(DBT* key, DBT* value, unsigned int flags);
	int sync(unsigned int flags = 0);
	int close(unsigned int flags = 0);

private:
	DB_ENV      *m_dbenv;
	DB          *m_dbp;
	DBC         *m_dbc;	
	DB_TXN      *m_txnid;

	char        *m_dbfile;
};

}

#endif


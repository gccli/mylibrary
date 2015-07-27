#ifndef MYSQL_CONN_H_
#define MYSQL_CONN_H_

#include "query.h"

namespace MySQL {

class Connection 
{
public:
    Connection(void);
    ~Connection(void);

    bool connect(const char* host, const char* user, const char* passwd, const char* db);
    bool disconnect(void);

    std::string  error(void);
    const std::string &version(void);

    Query *query(void);
    bool execute(const char *);

private:
	Query*		_query;
	std::string	m_version;
};

}

#endif

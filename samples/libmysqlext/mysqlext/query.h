#ifndef MYSQL_QUERY_H_
#define MYSQL_QUERY_H_

#include "row.h"

/*
 * I/O vector
 * write multiple buffers into database
 * this struct borrow ideas from linux iovec, readv and wirtev used
 */
struct mysql_iovec {
	void*					iov_base;  /* Starting address */
	size_t					iov_len;   /* Number of bytes */
};

namespace MySQL {

class Query 
{
    MySQL::Row*		_row;
	unsigned long	m_rows;
	MYSQL*			m_MySQL;
	MYSQL_RES*		m_result;
	std::string		m_strerr;

protected:
    friend class Connection;

    Query();
    ~Query();
	MYSQL* handle() { return m_MySQL; }

    bool init();
    bool deinit();

public:
	Row *fetch();
    bool reset();

    bool execute(const char *);
	bool execute(const char *, int);
	bool blob_write(const char* sql, const struct mysql_iovec* vector, int count);

	bool autocommit(bool on = true);
	bool commit();
    bool rollback();

    unsigned rows();
    std::string error(MYSQL_STMT* stmt = NULL);
};

}

#endif

#include "query.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

using namespace MySQL;

/*
 * This constructor is protected from general use, and is only
 * instantiated by the Connection object.
 */

Query::Query()
{
	_row		= NULL;
	m_rows		= 0;
	m_MySQL		= NULL;
	m_result	= NULL; 
}

/*
 * Deinit is hidden from public view, as is the destructor.  We only
 * want the main Connection object doing any instantiation or
 * destruction.
 */

Query::~Query(void)
{
	deinit();
}

bool Query::init()
{
	if (m_MySQL)
		return true;

	if ((m_MySQL = mysql_init (NULL)) == NULL){
		fprintf (stderr, "Initialize MySQL Handle Failed: No enough memory.\n");
		return false;
	}

	return true;
}

bool Query::deinit(void)
{
	if (!reset())
		return false;

	if (m_MySQL)
		mysql_close (m_MySQL);

	return true;
}

bool Query::autocommit(bool on)
{
	int ret = mysql_autocommit (m_MySQL, on);
	if (ret) 
		return false;

	return true;
}

bool Query::commit(void)
{
	return execute("COMMIT");
}

bool Query::rollback(void)
{
	return execute("ROLLBACK");
}

bool Query::reset(void) 
{
	if (_row) {
		delete _row;
		_row = NULL;
	}

	m_rows = 0;
	/*
	 * For security.
	 * Anyway, we would free result set in this place.
	 */
	if (m_result){
		mysql_free_result (m_result);
		m_result = NULL;
	}

	return true;
}

bool Query::execute(const char *s)
{
	return execute (s, (int )strlen(s));
}

bool Query::execute(const char* s, int len)
{
	if (!reset())
		return false;

	int ret = mysql_real_query(m_MySQL, s,(unsigned long) len);
	if (ret != 0) {
		fprintf (stderr, "Execute sql \"%s\" failed: \n%s\n", s, error().c_str());	
		return false;
	}
	else {
		m_rows = (unsigned long ) mysql_affected_rows (handle());
		if (m_rows == (unsigned long )~0){
			if (mysql_errno(handle()) == 0){			
			/*
			 * reset m_rows
			 * because of we call mysql_affected_rows before mysql_store_results
			 */
			m_rows = 0;
			return true;
			}
			fprintf (stderr, "mysql_affected_rows failed:\n%s\n", error().c_str());
		}
	}

	return true;
}

/*
 * If result set not null create and initialize _row object
 * each call fetch one row data until no data left
 * free the result set
 */

Row *Query::fetch(void) 
{
	/*
	 * TODO: we can use mysql_store_result
	 */

	if (!m_result){
		m_result	= mysql_store_result (handle());
		if (!m_result)
			return NULL;
	}

	m_rows = (unsigned long ) mysql_num_rows (m_result);

	if (!_row) {
		_row = new Row(m_result);

		if (!_row->init()) {
			delete _row;
			return _row = NULL;
		}
	}

	MYSQL_ROW r = mysql_fetch_row (m_result);
	if (r == NULL)
	{
		mysql_free_result (m_result);
		m_result = NULL;
		return NULL;
	}
	_row->reset();

	return _row->do_fetch(r);
}

bool Query::blob_write(const char* sql, const struct mysql_iovec* vector, int count)
{
	MYSQL_STMT* stmt;
	if ((stmt = mysql_stmt_init (handle())) == NULL) {
		fprintf (stderr, "Initialize stmt object failed: no enough memory!\n");
		return false;
	}

	if (mysql_stmt_prepare (stmt, sql, (unsigned long) strlen (sql))) {
		fprintf (stderr, "stmt prepare failed to execute \"%s\"\nERROR:%s\n",sql, error(stmt).c_str());
		return false;
	}

	const int bind_count = count;
	int param_cnt = mysql_stmt_param_count (stmt);
	if (param_cnt != bind_count) {
		fprintf (stderr, "Invalid parameters count %s.\n", error(stmt).c_str());
		return false;
	}
	
	MYSQL_BIND* bind = new MYSQL_BIND[bind_count];
	memset (bind, 0, bind_count*sizeof(MYSQL_BIND));
	for (int i=0; i<bind_count; ++i)
	{
		bind[i].buffer_type	= MYSQL_TYPE_BLOB;
		bind[i].buffer		= vector[i].iov_base;
		bind[i].length		= (unsigned long * )&vector[i].iov_len;
		bind[i].is_null		= 0;
	}

	if (mysql_stmt_bind_param (stmt, bind))	{
		fprintf (stderr, "stmt bind failed!\nERROR:%s\n", error(stmt).c_str());
		delete [] bind;
		return false;
	}
	
	if (mysql_stmt_execute (stmt)){
		fprintf (stderr, "stmt execute \"%s\" failed!\nERROR:%s\n",sql, error(stmt).c_str());
		delete [] bind;
		return false;		
	}

	delete [] bind;
	if (mysql_stmt_close (stmt)){
		fprintf (stderr, "stmt close failed!\nERROR:%s\n", error().c_str());
		return false;
	}

	return true;
}

unsigned Query::rows(void)
{
	return m_rows;
}

std::string Query::error(MYSQL_STMT* stmt)
{
	if (stmt){
		m_strerr = mysql_stmt_error (stmt);
	}
	else {
		m_strerr = mysql_error (handle());
	}

	return m_strerr;
}

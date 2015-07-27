#ifndef MYSQL_ROW_H_
#define MYSQL_ROW_H_

#include <vector>
#include "field.h"

namespace MySQL {

class Row 
{
public:
	typedef std::vector<Field *> fieldlist_t;

private:
    fieldlist_t	_columns;
	MYSQL_RES*	&m_result;
    Field*		EMPTY;

protected:
    friend class Query;

    Row(MYSQL_RES *&);
    ~Row(void);

	Row* do_fetch(MYSQL_ROW& r);
	bool init(void);
    bool deinit(void);

    Field *field(const char *);

public:
    unsigned width(void);
    int position(const char *);
    const char *name(unsigned);
    bool isnull(unsigned);

    bool reset(void);

    Field& operator[] (signed);
    Field& operator[] (unsigned);
    Field& operator[] (std::string &);
    Field& operator[] (const char *);
};

}

#endif

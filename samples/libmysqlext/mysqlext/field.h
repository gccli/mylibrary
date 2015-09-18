#ifndef MYSQL_FIELD_H_
#define MYSQL_FIELD_H_

#ifdef WIN32
	#include <winsock2.h>
	#pragma comment (lib, "ws2_32")
	#pragma warning (disable:4996)
	#define strcasecmp _stricmp
#endif //WIN32

extern "C" {
	#include <mysql.h>
}
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace MySQL {

class Field 
{
protected:
    friend class Row;

	unsigned			width;
	bool				is_null;
	enum_field_types	type;    // MySQL actual type
	char*				value;

    Field(const char [], unsigned, unsigned width);
    ~Field(void);

public:
    std::string			name;
	unsigned			length;	

    /*
     * Bunch of type conversions for individual fields.
     */
    operator char(void);
    operator char*(void);
    operator const char *(void);
    operator int(void);
    operator unsigned(void);
    operator long(void);
    operator unsigned long(void);
};

}

#endif

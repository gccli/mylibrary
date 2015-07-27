#ifndef _MYAPP_COMM_H__
#define _MYAPP_COMM_H__

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ldap.h>

#include "debug.h"

extern const char *my_uri;
extern const char *my_dn;
extern const char *my_pwd;


int LdapInit();

#endif

#include <iostream>
#include <string>
#include "comm.h"

using namespace std;

const char *my_uri="ldaps://ldap0.inetlinux.net";
const char *my_dn = "cn=admin,dc=inetlinux,dc=net";
const char *my_pwd = "cc";

int main(int argc, char *argv[])
{
    int ret,msgid;
    LDAP *handle = NULL;
    LDAPMessage *result = NULL, *msg;
    LDAPControl     **sctrlsp = NULL;
    struct berval *cred, *rcred;
    int protocol = LDAP_VERSION3;
    struct timeval tv = {3,0};

    ret = ldap_initialize(&handle, my_uri);
    if (ret != LDAP_SUCCESS) {
	cout <<"ldap not initialized\n";
	goto err;
    }

    ret = ldap_set_option(handle, LDAP_OPT_PROTOCOL_VERSION, &protocol);
    if (ret != LDAP_SUCCESS) {
	debugs(1, "ldap set option - %s\n", ldap_err2string(ret));
	goto err;
    }

    cred = (struct berval *)calloc(1, sizeof(*cred));
    rcred = NULL;
    cred->bv_val = (char*)my_pwd;
    cred->bv_len = strlen(my_pwd);

    ret = ldap_sasl_bind(handle,NULL,LDAP_SASL_SIMPLE,cred,sctrlsp,NULL, &msgid);
    if (ret != LDAP_SUCCESS) {
	debugs(1, "ldap bind failed - %s\n", ldap_err2string(ret));
	goto err;
    }

    ret = ldap_result(handle, msgid, 1, NULL, &result);
    if (ret < 0) {
	debugs(1, "ldap bind failed - %s\n", ldap_err2string(ret));
	goto err;
    }
    debugs(3, "ldap bind result %d protocol %d\n", ret, protocol);

    do {
	char *attrs[] = {NULL};
	char *base = NULL;
	char *filter = NULL;
	int  scope = 0;
	int  attrsonly = 0;
	int  sizelimit = 0;
	ret = ldap_search_ext(handle,
			      base,
			      scope,
			      filter,
			      attrs,
			      attrsonly,
			      NULL,
			      NULL,
			      &tv,
			      sizelimit,
			      &msgid);
	if (ret != LDAP_SUCCESS) {
	    cout <<"ldap search failed - " << ldap_err2string(ret) << '\n';
	    goto err;
	} 

	while ((ret = ldap_result(handle, LDAP_RES_ANY,
				  1?LDAP_MSG_ALL:LDAP_MSG_ONE,
				  &tv, &result )) > 0 ) {

	    for ( msg = ldap_first_message(handle, result);
		  msg != NULL;
		  msg = ldap_next_message(handle, msg) )
	    {
		cout <<"got msg, type " << ldap_msgtype(msg) << '\n';
		
	    }
    }

    }while(0);

err:
    return ret;
}

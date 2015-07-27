#include "comm.h"

int LdapInit()
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

    ret = 0;
err:
    return ret;
}

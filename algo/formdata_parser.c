#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_WF_KEY_LEN          128
#define MAX_WF_VAL_LEN         1024

typedef enum form_state_e {
    CHAR_KEY,
    CHAR_ASSIGN,
    CHAR_VALUE,
    CHAR_AT,
    CHAR_END
} form_state_t;

// Web formdata parser

typedef struct form_parser_s {
    form_state_t     state;
    int              key_len;
    int              val_len;
    u_char           key[MAX_WF_KEY_LEN];
    u_char           val[MAX_WF_VAL_LEN];
} form_parser_t;

typedef struct {
    size_t      len;
    u_char     *data;
} ngx_str_t;

int sf_parse_form(form_parser_t *fp, u_char *data, u_char *end,
                  ngx_str_t *key, ngx_str_t *val)
{
    u_char *p;
    int     stop = 0;
    int     i, j;

    i = fp->key_len;
    j = fp->val_len;

    for (p = data; !stop && p<end; ++p) {
        switch(fp->state) {
        case CHAR_KEY:
            if (*p == '=') {
                fp->state = CHAR_ASSIGN;
            } else if (i < MAX_WF_KEY_LEN) {
                fp->key[i++] = *p;
            }
            break;
        case CHAR_ASSIGN:
            fp->key_len = i;
            j = 0;
            fp->state = CHAR_VALUE;
            p--;   // repeat for retrieve value

            break;
        case CHAR_VALUE:
            if (*p == '&') {
                fp->state = CHAR_AT;
            } else if (j<MAX_WF_VAL_LEN) {
                fp->val[j++] = *p;
            }
            break;
        case CHAR_AT:
            fp->val_len = j;
            i = 0;
            fp->state = CHAR_KEY;
            p--;   // repeat for retrieve key
            if (memcmp(fp->key, key->data, fp->key_len) == 0) {
                val->data = fp->val; val->len = fp->val_len;
                stop = 1;
            }
            printf("  [%.*s]=[%.*s]\n", fp->key_len, fp->key, fp->val_len, fp->val);

            break;
        case CHAR_END:
            stop = 1;
            break;
        }
    }

   fp->key_len = i;
   fp->val_len = j;

   return stop;
}

void find_key(const char *keyid, u_char *data, size_t len)
{
    int           got = 0;
    size_t        i, interval;
    ngx_str_t     key, value;
    form_parser_t wfp;


    key.data = (u_char *)keyid;
    key.len = strlen(keyid);

    memset(&wfp, 0, sizeof(wfp));

    interval = 20;
    for(i=0; i<len; i+=interval) {
        if (sf_parse_form(&wfp, (u_char *)(data+i), (u_char *)(data+i+interval),
                          &key, &value)) {
            got = 1;
            break;
        }
    }
    if (!got && len - i > 0) {
        i -= interval;
        if (sf_parse_form(&wfp, (u_char *)(data+i), (u_char *)(data+len),
                          &key, &value)) {
            got = 1;
        }
    }
    if (got)printf("GOT %s=%.*s@\n", keyid, (int)value.len, value.data);
}

int main(int argc, char *argv[])
{
    const char *data = "entityId=0012800000gxhO9&sysMod=1555d7464c8&_CONFIRMATIONTOKEN=VmpFPSxNakF4Tmkwd05pMHlObFF4TURvMU5qb3lOeTR5TmpSYSw0djdNN2R6VlhseGJFTnFmeVhSdEhFLE9EUm1OMkV4&save=1&acc7=Electronics&00N2800000CSGj2=&00N2800000CSGj5=&00N2800000CSGj6=&00N2800000CSGj7=&00N2800000CSGj3=&00N2800000CSGj1=&00N2800000CSGj4=&acc2=Edge%20Communications&acc3=&acc3_lkid=000000000000000&acc3_lkold=&acc5=CD451796&acc12=http%3A%2F%2Fedgecomm.com&acc16=6576&acc23=&acc6=Customer%20-%20Direct&acc8=0&acc14=Public&acc15=1%2C000&acc9=Hot&acc20=Edge%2C%20founded%20in%201998%2C%20is%20a%20start-up%20based%20in%20Austin%2C%20TX.%20The%20company%20designs%20and%20manufactures%20a%20device%20to%20convert%20music%20from%20one%20digital%20format%20to%20another.%20Edge%20sells%20its%20product%20through%20retailers%20and%20its%20own%20website.&acc13=EDGE&acc10=(512)%20757-6000&acc17street=312%20Constitution%20Place%20Austin%2C%20TX%2078767%20USA&acc17city=Austin&acc17zip=&acc17state=TX&acc17country=&acc18street=312%20Constitution%20Place%20Austin%2C%20TX%2078767%20USA&acc18city=&acc18zip=&acc18state=&acc18country=&acc11=(512)%20757-9000";

    find_key("entityId",           (u_char *)data, strlen(data));
    find_key("city",               (u_char *)data, strlen(data));
    find_key("00N2800000CSGj6",    (u_char *)data, strlen(data));
    find_key("00N2800000CSGj7",    (u_char *)data, strlen(data));
    find_key("acc18zip",           (u_char *)data, strlen(data));
    find_key("save",               (u_char *)data, strlen(data));
    find_key("_CONFIRMATIONTOKEN", (u_char *)data, strlen(data));
    find_key("acc11",              (u_char *)data, strlen(data));

    return 0;
}

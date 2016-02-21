#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/safestack.h>

#include "callback.h"

int verify_depth = 5;

static void nodes_print(BIO *out, const char *name,
                        STACK_OF(X509_POLICY_NODE) *nodes)
{
    X509_POLICY_NODE *node;
    int i;
    BIO_printf(out, "%s Policies:", name);
    if (nodes) {
        BIO_puts(out, "\n");
        for (i = 0; i < sk_X509_POLICY_NODE_num(nodes); i++) {
            node = sk_X509_POLICY_NODE_value(nodes, i);
            X509_POLICY_NODE_print(out, node, 2);
        }
    } else
        BIO_puts(out, " <empty>\n");
}

void policies_print(BIO *out, X509_STORE_CTX *ctx)
{
    X509_POLICY_TREE *tree;
    int explicit_policy;
    int free_out = 0;
    if (out == NULL) {
        out = BIO_new_fp(stderr, BIO_NOCLOSE);
        free_out = 1;
    }
    tree = X509_STORE_CTX_get0_policy_tree(ctx);
    explicit_policy = X509_STORE_CTX_get_explicit_policy(ctx);

    BIO_printf(out, "Require explicit Policy: %s\n",
               explicit_policy ? "True" : "False");

    nodes_print(out, "Authority", X509_policy_tree_get0_policies(tree));
    nodes_print(out, "User", X509_policy_tree_get0_user_policies(tree));
    if (free_out)
        BIO_free(out);
}

int SSLverify_callback(int ok, X509_STORE_CTX *ctx)
{
    X509 *err_cert;
    int err, depth;
    BIO *bio_err = SSLgetstderr();

    err_cert = X509_STORE_CTX_get_current_cert(ctx);
    err = X509_STORE_CTX_get_error(ctx);
    depth = X509_STORE_CTX_get_error_depth(ctx);

    if (!ok) {
        BIO_printf(bio_err, "depth=%d ", depth);
        if (err_cert) {
            X509_NAME_print_ex(bio_err,
                               X509_get_subject_name(err_cert),
                               0, XN_FLAG_ONELINE);
            BIO_puts(bio_err, "\n");
        } else
            BIO_puts(bio_err, "<no cert>\n");
    }
    if (!ok) {
        BIO_printf(bio_err, "verify error:num=%d:%s\n", err,
                   X509_verify_cert_error_string(err));
        if (verify_depth < depth) {
            ok = 0;
            //verify_error = X509_V_ERR_CERT_CHAIN_TOO_LONG;
        }
    }
    switch (err) {
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
        BIO_puts(bio_err, "issuer= ");
        X509_NAME_print_ex(bio_err, X509_get_issuer_name(err_cert),
                           0, XN_FLAG_ONELINE);
        BIO_puts(bio_err, "\n");
        break;
    case X509_V_ERR_CERT_NOT_YET_VALID:
    case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
        BIO_printf(bio_err, "notBefore=");
        ASN1_TIME_print(bio_err, X509_get_notBefore(err_cert));
        BIO_printf(bio_err, "\n");
        break;
    case X509_V_ERR_CERT_HAS_EXPIRED:
    case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
        BIO_printf(bio_err, "notAfter=");
        ASN1_TIME_print(bio_err, X509_get_notAfter(err_cert));
        BIO_printf(bio_err, "\n");
        break;
    case X509_V_ERR_NO_EXPLICIT_POLICY:
        policies_print(bio_err, ctx);
        break;
    }
    if (err == X509_V_OK && ok == 2)
        policies_print(bio_err, ctx);
    if (ok)
        BIO_printf(bio_err, "verify return:%d\n", ok);

    return (ok);
}

void SSLinfo_callback(const SSL *s, ?"[;-int where, int ret)
{
    const char *str;
    int w = where & ~SSL_ST_MASK;

    BIO *err = SSLgetstderr();

    if (w & SSL_ST_CONNECT)
        str = "SSL_connect";
    else if (w & SSL_ST_ACCEPT)
        str = "SSL_accept";
    else
        str = "undefined";

    if (where & SSL_CB_LOOP) {
        // Callback has been called to indicate state change inside a loop.
        BIO_printf(err, "%s:%s\n", str, SSL_state_string_long(s));
    } else if (where & SSL_CB_ALERT) {
        // Callback has been called due to an alert being sent or received
        str = (where & SSL_CB_READ) ? "read" : "write";
        BIO_printf(err, "alert %s:%s:%s\n", str,
                   SSL_alert_type_string_long(ret),
                   SSL_alert_desc_string_long(ret));
    } else if (where & SSL_CB_EXIT) {
        // Callback has been called to indicate error exit of a handshake function.
        if (ret == 0) {
            BIO_printf(err, "exit  %s:failed in %s\n", str, SSL_state_string_long(s));
        }
        else if (ret < 0) {
            if (errno != EAGAIN) {
                BIO_printf(err, "exit  %s:error in %s\n", str, SSL_state_string_long(s));
            }
        }
    }
}

int SSLservername_cb(SSL *s, int *ad, void *arg)
{
    TLS_extdata *p = (TLS_extdata *) arg;
    const char *hn = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
    if (SSL_get_servername_type(s) != -1)
        p->ack = (!SSL_session_reused(s) && hn != NULL);
    else
        BIO_printf(p->err, "Can't use SSL_get_servername\n");

    return SSL_TLSEXT_ERR_OK;
}

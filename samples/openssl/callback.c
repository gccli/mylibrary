#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/safestack.h>

#include "callback.h"

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
    X509 *cert;
    int err, depth;
    BIO *bio_err = SSLgetstderr();

    CCTX_extdata_t *ext;

    cert = X509_STORE_CTX_get_current_cert(ctx);
    err = X509_STORE_CTX_get_error(ctx);
    depth = X509_STORE_CTX_get_error_depth(ctx);


    //printf("%p ");

    //SSL_CTX_get_ex_data(ctx, SSL_CTX_extdata_index);

    if (!ok) {
        BIO_printf(bio_err, "depth=%d ", depth);
        if (cert) {
            X509_NAME_print_ex(bio_err,
                               X509_get_subject_name(cert),
                               0, XN_FLAG_ONELINE);
            BIO_puts(bio_err, "\n");
        } else
            BIO_puts(bio_err, "<no cert>\n");
    }
    if (!ok) {
        BIO_printf(bio_err, "verify error:num=%d:%s\n", err,
                   X509_verify_cert_error_string(err));

        //if (ext->verify_ignore)
        //ok = 1;
    }
    switch (err) {
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
        BIO_puts(bio_err, "issuer= ");
        X509_NAME_print_ex(bio_err, X509_get_issuer_name(cert),
                           0, XN_FLAG_ONELINE);
        BIO_puts(bio_err, "\n");
        break;
    case X509_V_ERR_CERT_NOT_YET_VALID:
    case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
        BIO_printf(bio_err, "notBefore=");
        ASN1_TIME_print(bio_err, X509_get_notBefore(cert));
        BIO_printf(bio_err, "\n");
        break;
    case X509_V_ERR_CERT_HAS_EXPIRED:
    case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
        BIO_printf(bio_err, "notAfter=");
        ASN1_TIME_print(bio_err, X509_get_notAfter(cert));
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

static void print_server_cert(const SSL *ssl, BIO *errbio)
{
    char tmp[256] = {0};
    X509 *peer = SSL_get_peer_certificate(ssl);
    if (!peer) {
        return ;
    }

    PEM_write_bio_X509(errbio, peer);
    X509_NAME_oneline(X509_get_subject_name(peer), tmp, sizeof(tmp));
    BIO_printf(errbio, "subject=%s\n", tmp);
    X509_NAME_oneline(X509_get_issuer_name(peer), tmp, sizeof(tmp));
    BIO_printf(errbio, "issuer=%s\n", tmp);
}

#define CASE(str, x) case (x):                          \
    if (str && str[0]) BIO_printf(err, str "\n");       \
    else BIO_printf(err, #x"\n" );

void SSLinfo_callback(const SSL *s, int where, int ret)
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
        BIO_printf(err, "SSL state:%x %s:%s\n", s->state, str, SSL_state_string_long(s));
        switch(s->state) {
/* CLIENT */
        CASE("", SSL3_ST_CW_FLUSH)
            break;
// Write to server
        CASE("", SSL3_ST_CW_CLNT_HELLO_A)
            break;
        CASE("", SSL3_ST_CW_CLNT_HELLO_B)
            break;
// Read from server
        CASE("", SSL3_ST_CR_SRVR_HELLO_A)
            break;
        CASE("", SSL3_ST_CR_SRVR_HELLO_B)
            break;
        CASE("Server Certificate", SSL3_ST_CR_CERT_A)
            print_server_cert(s, err);
            break;
        CASE("", SSL3_ST_CR_CERT_B)
            break;
        CASE("", SSL3_ST_CR_KEY_EXCH_A)
            break;
        CASE("", SSL3_ST_CR_KEY_EXCH_B)
            break;
        CASE("", SSL3_ST_CR_CERT_REQ_A)
            break;
        CASE("", SSL3_ST_CR_CERT_REQ_B)
            break;
        CASE("", SSL3_ST_CR_SRVR_DONE_A)
            break;
        CASE("", SSL3_ST_CR_SRVR_DONE_B)
            break;
// Write to server
        CASE("", SSL3_ST_CW_CERT_A)
            break;
        CASE("", SSL3_ST_CW_CERT_B)
            break;
        CASE("", SSL3_ST_CW_CERT_C)
            break;
        CASE("", SSL3_ST_CW_CERT_D)
            break;
        CASE("", SSL3_ST_CW_KEY_EXCH_A)
            break;
        CASE("", SSL3_ST_CW_KEY_EXCH_B)
            break;
        CASE("", SSL3_ST_CW_CERT_VRFY_A)
            break;
        CASE("", SSL3_ST_CW_CERT_VRFY_B)
            break;
        CASE("", SSL3_ST_CW_CHANGE_A)
            break;
        CASE("", SSL3_ST_CW_CHANGE_B)
            break;
        CASE("", SSL3_ST_CW_NEXT_PROTO_A)
            break;
        CASE("", SSL3_ST_CW_NEXT_PROTO_B)
            break;
        CASE("", SSL3_ST_CW_FINISHED_A)
            break;
        CASE("", SSL3_ST_CW_FINISHED_B)
            break;
// Read from server
        CASE("", SSL3_ST_CR_CHANGE_A)
            break;
        CASE("", SSL3_ST_CR_CHANGE_B)
            break;
        CASE("", SSL3_ST_CR_FINISHED_A)
            break;
        CASE("", SSL3_ST_CR_FINISHED_B)
            break;
        CASE("", SSL3_ST_CR_SESSION_TICKET_A)
            break;
        CASE("", SSL3_ST_CR_SESSION_TICKET_B)
            break;
        CASE("", SSL3_ST_CR_CERT_STATUS_A)
            break;
        CASE("", SSL3_ST_CR_CERT_STATUS_B)
            break;

/* SERVER */
        CASE("", SSL3_ST_SW_FLUSH)
            break;
// Read from client
        CASE("", SSL3_ST_SR_CLNT_HELLO_A)
            break;
        CASE("", SSL3_ST_SR_CLNT_HELLO_B)
            break;
        CASE("", SSL3_ST_SR_CLNT_HELLO_C)
            break;
        CASE("", SSL3_ST_SR_CLNT_HELLO_D)
            break;
// Write to client
        CASE("", DTLS1_ST_SW_HELLO_VERIFY_REQUEST_A)
            break;
        CASE("", DTLS1_ST_SW_HELLO_VERIFY_REQUEST_B)
            break;
        CASE("", SSL3_ST_SW_HELLO_REQ_A)
            break;
        CASE("", SSL3_ST_SW_HELLO_REQ_B)
            break;
        CASE("", SSL3_ST_SW_HELLO_REQ_C)
            break;
        CASE("", SSL3_ST_SW_SRVR_HELLO_A)
            break;
        CASE("", SSL3_ST_SW_SRVR_HELLO_B)
            break;
        CASE("", SSL3_ST_SW_CERT_A)
            break;
        CASE("", SSL3_ST_SW_CERT_B)
            break;
        CASE("", SSL3_ST_SW_KEY_EXCH_A)
            break;
        CASE("", SSL3_ST_SW_KEY_EXCH_B)
            break;
        CASE("", SSL3_ST_SW_CERT_REQ_A)
            break;
        CASE("", SSL3_ST_SW_CERT_REQ_B)
            break;
        CASE("", SSL3_ST_SW_SRVR_DONE_A)
            break;
        CASE("", SSL3_ST_SW_SRVR_DONE_B)
            break;
// Read from client
        CASE("", SSL3_ST_SR_CERT_A)
            break;
        CASE("", SSL3_ST_SR_CERT_B)
            break;
        CASE("", SSL3_ST_SR_KEY_EXCH_A)
            break;
        CASE("", SSL3_ST_SR_KEY_EXCH_B)
            break;
        CASE("", SSL3_ST_SR_CERT_VRFY_A)
            break;
        CASE("", SSL3_ST_SR_CERT_VRFY_B)
            break;
        CASE("", SSL3_ST_SR_CHANGE_A)
            break;
        CASE("", SSL3_ST_SR_CHANGE_B)
            break;
# ifndef OPENSSL_NO_NEXTPROTONEG
        CASE("", SSL3_ST_SR_NEXT_PROTO_A)
            break;
        CASE("", SSL3_ST_SR_NEXT_PROTO_B)
            break;
# endif
        CASE("", SSL3_ST_SR_FINISHED_A)
            break;
        CASE("", SSL3_ST_SR_FINISHED_B)
            break;
/* write to client */
        CASE("", SSL3_ST_SW_CHANGE_A)
            break;
        CASE("", SSL3_ST_SW_CHANGE_B)
            break;
        CASE("", SSL3_ST_SW_FINISHED_A)
            break;
        CASE("", SSL3_ST_SW_FINISHED_B)
            break;
        CASE("", SSL3_ST_SW_SESSION_TICKET_A)
            break;
        CASE("", SSL3_ST_SW_SESSION_TICKET_B)
            break;
        CASE("", SSL3_ST_SW_CERT_STATUS_A)
            break;
        CASE("", SSL3_ST_SW_CERT_STATUS_B)
            break;
        }
    } else if (where & SSL_CB_ALERT) {
        // Callback has been called due to an alert being sent or received
        str = (where & SSL_CB_READ) ? "read" : "write";
        BIO_printf(err, "A: ret:%d %s:%s:%s\n", ret, str, SSL_alert_type_string_long(ret),
                   SSL_alert_desc_string_long(ret));
    } else if (where & SSL_CB_EXIT) {
        // Callback has been called to indicate error exit of a handshake function.
        if (ret <= 0 && errno != EAGAIN) {
            BIO_printf(err, "E: ret:%d, %s:%s\n", ret, str, SSL_state_string_long(s));
        }
    }
}

int SSLservername_cb(SSL *s, int *ad, void *arg)
{
    const char *hn;

    BIO *err = SSLgetstderr();
    hn = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
    BIO_printf(err, "server name: %s\n", hn);
    if (SSL_get_servername_type(s) != -1) {
        //p->ack = (!SSL_session_reused(s) && hn != NULL);
    }
    else
        BIO_printf(err, "Can't use SSL_get_servername\n");

    return SSL_TLSEXT_ERR_OK;
}

#ifndef RSA_KEY_MGR_H__
#define RSA_KEY_MGR_H__

#include <openssl/evp.h>

/**
 * Generate RSA key pair
 * return the generated RSA key, the type specified by EVP_PKEY
 */
EVP_PKEY *rsa_gen_key();

/**
 * Export RSA private key to file @prikey
 * The passphrase @pass used for encrypt/decrypt private key, the @pass
 * parameter should be NULL-terminated string
 */
int export_key(EVP_PKEY *pk, const char *prikey, unsigned char *pass);

/**
 * Export RSA public key to file @pubkey
 */
int export_pubkey(EVP_PKEY *pk, const char *pubkey);

/**
 * Load private/public key from file @ifile, using the passphrase @pass for
 * decrypt private key
 */
EVP_PKEY *rsa_load_key(const char *ifile, unsigned char *pass);
EVP_PKEY *rsa_load_pubkey(const char *ifile);
int rsa_dec_enc(EVP_PKEY *pk, int enc, unsigned char *in, size_t inlen,
                unsigned char **outp, size_t *outl);


#endif

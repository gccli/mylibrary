#ifndef RSA_KEY_MGR_H__
#define RSA_KEY_MGR_H__

#include "cipher.h"

class rsakey {
public:
    rsakey();
    virtual ~rsakey();

public:
/**
 * Export rsa key to @file, default public key will exported and @pass will be
 * ignored, if @pub set to 0 private key will be export and private key was
 * encrypted by @pass
 */
    int export_key(const char *ofile, int pub=1, unsigned char *pass=NULL);
    int load_key(const char *ifile, unsigned char *pass);
    int load_pubkey(const char *ifile);

    int enc_dec(unsigned char *in, size_t inlen,unsigned char **outp,
                size_t *outl, int dec = 0);

public:
/**
 * Generate RSA key pair
 * return the generated RSA key, the type specified by EVP_PKEY
 */
    static int generate(EVP_PKEY **pk);
/**
 * Export RSA public key to file @file
 */
    static int export_pubkey(EVP_PKEY *pk, const char *file);
/**
 * Export RSA private key to file @prikey
 * The passphrase @pass used for encrypt/decrypt private key, the @pass
 * parameter should be NULL-terminated string
 */
    static int export_key(EVP_PKEY *pk, const char *file, unsigned char *pass);

private:
    EVP_PKEY *pk_pri;
    EVP_PKEY *pk_pub;
};


#endif

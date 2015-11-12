Security programming with OpenSSL
=================================

This page all about openssl programming


Message Digest
--------------

$ openssl dgst -sha1 openssl.txt
$ openssl dgst -sha1 -out s.txt openssl.txt

Symmetric Ciphers
-----------------
[Symmetric-key algorithms](https://en.wikipedia.org/wiki/Symmetric-key_algorithm) are algorithms for cryptography that use the **same cryptographic keys** for both encryption of plaintext and decryption of ciphertext.

Symmetric-key encryption can use either stream ciphers or [block ciphers](https://en.wikipedia.org/wiki/Block_cipher)
A **stream cipher** is one that encrypts a digital data stream one bit or one byte at a time.
A **block cipher** is a deterministic algorithm operating on fixed-length groups of bits, called *blocks*, with an unvarying transformation that is specified by a symmetric key.

    openssl enc -aes-256-cbc -salt -in openssl.txt -out openssl.bin
    openssl enc -aes-256-cbc -d -in openssl.bin -pass pass:passwd

### Block cipher ###

#### [Feistel cipher](https://en.wikipedia.org/wiki/Feistel_cipher) ####

#### Block cipher mode of operation ####
Most modes require a unique binary sequence, often called an initialization vector (IV), for each encryption operation.
IV is a block of bits that is used by several modes to randomize the encryption and hence to produce distinct ciphertexts even if the same plaintext is encrypted multiple times

ECB, CBC, OFB, and CFB


Public Key Cryptography
-----------------------

in order for a server to employ the SSL protocol, it requires a private key and a certificate.
The certificate contains the public key that matches the server's private key

  Diffie-Hellman
  Diffie-Hellman is used for key agreement. In simple terms, key agreement is the exchange of
  information over an insecure medium that allows each of the two parties in a conversation to
  compute a value that is typically used as the key for a symmetric cipher.
  $ openssl dhparam -out dhparam.pem -2 1024
  $ openssl dhparam -in dhparam.pem -noout -C
    Reads a set of Diffie-Hellman parameters from the file dhparam.pem and writes a C code
    representation of the parameters to stdout

  Digital Signature Algorithm
  $ openssl dsaparam -out dsaparam.pem 1024
  $ openssl gendsa -out dsaprivatekey.pem -des3 dsaparam.pem
  $ openssl dsa -in dsaprivatekey.pem -pubout -out dsapublickey.pem
  $ openssl dsa -in dsaprivatekey.pem -out dsaprivatekey.pem -des3 -passin pass:oldpword -passout pass:newpword

  RSA
  RSA provides secrecy, authentication, and encryption all in one neat little package
  Generation of an RSA private key involves finding two large prime numbers, each approximately half
  the length of the key
  $ openssl genrsa -out rsaprivatekey.pem -passout pass:security -des3 1024
  $ openssl rsa -in rsaprivatekey.pem -passin pass:security -pubout -out rsapublickey.pem
  $ openssl rsautl -encrypt -pubin -inkey rsapublickey.pem -in plain.txt -out cipher.txt
  $ openssl rsautl -decrypt -inkey rsaprivatekey.pem -in cipher.txt -out plain.txt
  $ openssl rsautl -sign -inkey rsaprivatekey.pem -in plain.txt -out signature.bin
  $ openssl rsautl -verify -pubin -inkey rsapublickey.pem -in signature.bin -out plain.txt


PUBLIC KEY INFRASTRUCTURE
-------------------------

public key cryptography provides no means of establishing trust when used on its own.

### Certificates ###

a certificate binds a public key with a distinguished name.
  1. Certification Authorities
  private CA and public CA
  2. Certificate Hierarchies
  3. Certificate Extensions
  The most widely accepted format for certificates is the X.509 format,
  The X.509v3 standard defines 14 extensions in an effort to consolidate the most common
  extensions implemented by third parties
  4. Certificate Revocation Lists
  5. Online Certificate Status Protocol

Obtaining a Certificate
  1. Personal Certificates
  2. Code-Signing Certificates
  3. Web Site Certificates

Setting Up a Certification Authority


### SSL/TLS Programming ###

Background
struct SSL_METHOD, SSL_CTX, and SSL
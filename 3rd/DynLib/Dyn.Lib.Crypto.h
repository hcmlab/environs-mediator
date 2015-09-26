/**
 * Dynamically accessing openssl
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
 * @remarks
 *
 * This file is part of the Environs framework developed at the
 * Lab for Human Centered Multimedia of the University of Augsburg.
 * http://hcm-lab.de/environs
 *
 * Environ is free software; you can redistribute it and/or modify
 * it under the terms of the Eclipse Public License v1.0.
 * A copy of the license may be obtained at:
 * http://www.eclipse.org/org/documents/epl-v10.html
 * --------------------------------------------------------------------
 */
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_DYNAMIC_OPENSSL_H
#define INCLUDE_HCM_ENVIRONS_DYNAMIC_OPENSSL_H

#include "Interop/Export.h"
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/bn.h>

namespace environs
{
    typedef void (CallConv * pOPENSSL_add_all_algorithms_noconf)(void);
    typedef void (CallConv * pOPENSSL_add_all_algorithms_conf)(void);
    
    typedef void (CallConv * pERR_load_ERR_strings)(void);
    typedef void (CallConv * pERR_load_crypto_strings)(void);
    
    typedef int (CallConv * pEVP_PKEY_set1_RSA)(EVP_PKEY *pkey, struct rsa_st *key);
    typedef RSA * (CallConv * pd2i_RSA_PUBKEY)(RSA **a, const unsigned char **pp, long length);
    
    typedef X509 * (CallConv * pPEM_read_X509)(FILE *fp, X509 **x, pem_password_cb *cb, void *u);
    typedef RSA * (CallConv * pPEM_read_RSAPrivateKey)(FILE *fp, RSA **x, pem_password_cb *cb, void *u);
    
    
    typedef EVP_PKEY * (CallConv * pEVP_PKEY_new)(void);
    typedef void (CallConv * pX509_free)(X509 *a);
    typedef void (CallConv * pEVP_PKEY_free)(EVP_PKEY *pkey);
    
    typedef const EVP_MD * (CallConv * pEVP_sha1)(void);

    typedef void (CallConv * pRSA_free)(RSA *a);
    
    typedef BIGNUM * (CallConv * pBN_new)(void);
    typedef void (CallConv * pBN_free)(BIGNUM *a);
    
    typedef int (CallConv * pBN_set_word)(BIGNUM *a, BN_ULONG w);
    
    typedef RSA * (CallConv * pRSA_new)(void);
    
    typedef RSA * (CallConv * pRSA_generate_key)(int bits, unsigned long e, void
                                                 (*callback) (int, int, void *), void *cb_arg);
    
    typedef int (CallConv * pRSA_generate_key_ex)(RSA *rsa, int bits, BIGNUM *e, BN_GENCB *cb);
    
    typedef int (CallConv * pEVP_PKEY_assign)(EVP_PKEY *pkey, int type, void *key);
    
    typedef int (CallConv * pASN1_INTEGER_set)(ASN1_INTEGER *a, long v);
    typedef ASN1_TIME * (CallConv * pX509_gmtime_adj)(ASN1_TIME *s, long adj);
    
    typedef X509 * (CallConv * pX509_new)(void);
    
    typedef int (CallConv * pX509_set_pubkey)(X509 *x, EVP_PKEY *pkey);
    typedef X509_NAME * (CallConv * pX509_get_subject_name)(X509 *a);
    
    typedef int (CallConv * pX509_NAME_add_entry_by_txt)(X509_NAME *name, const char *field, int type,
                                   const unsigned char *bytes, int len, int loc,
                                   int set);
    
    typedef int (CallConv * pX509_set_issuer_name)(X509 *x, X509_NAME *name);
    
    typedef ASN1_INTEGER * (CallConv * pX509_get_serialNumber)(X509 *x);
    
    typedef int (CallConv * pX509_sign)(X509 *x, EVP_PKEY *pkey, const EVP_MD *md);
    
    typedef RSA * (CallConv * pd2i_RSAPrivateKey)(RSA **a, const unsigned char **in, long len);
    typedef int (CallConv * pi2d_RSAPrivateKey)(const RSA *a, unsigned char **out);

    typedef int (CallConv * pi2d_X509)(const X509 *a, unsigned char **out);
    
    typedef X509 * (CallConv * pd2i_X509)(X509 **a, const unsigned char **in, long len);
    typedef EVP_PKEY * (CallConv * pX509_get_pubkey)(X509 *x);
    
    typedef struct rsa_st * (CallConv * pEVP_PKEY_get1_RSA)(EVP_PKEY *pkey);
    typedef int (CallConv * pRSA_size)(const RSA *rsa);
    
    typedef void (CallConv * pERR_print_errors_fp)(FILE *fp);
    
    typedef int (CallConv * pRSA_public_encrypt)(int flen, const unsigned char *from,
                                                 unsigned char *to, RSA *rsa, int padding);
    
    typedef int (CallConv * pRSA_private_decrypt)(int flen, const unsigned char *from,
                                                  unsigned char *to, RSA *rsa, int padding);
    
    typedef int (CallConv * pEVP_CIPHER_CTX_cleanup)(EVP_CIPHER_CTX *a);
    typedef void (CallConv * pEVP_CIPHER_CTX_init)(EVP_CIPHER_CTX *a);
    
    typedef int (CallConv * pEVP_EncryptInit_ex)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                                                 ENGINE *impl, const unsigned char *key,
                                                 const unsigned char *iv);
    
    
    typedef int (CallConv * pEVP_DecryptInit_ex)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                                                 ENGINE *impl, const unsigned char *key,
                                                 const unsigned char *iv);
    
    typedef int (CallConv * pEVP_EncryptUpdate)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl,
                                                const unsigned char *in, int inl);
    
    typedef int (CallConv * pEVP_EncryptFinal_ex)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl);
    
    typedef int (CallConv * pEVP_DecryptUpdate)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl,
                                                const unsigned char *in, int inl);
    
    typedef int (CallConv * pEVP_DecryptFinal_ex)(EVP_CIPHER_CTX *ctx, unsigned char *outm, int *outl);
    
    typedef const EVP_CIPHER * (CallConv * pEVP_aes_256_cbc)(void);
    
    typedef int (CallConv * pSHA256_Init)(SHA256_CTX *c);
    
    typedef int (CallConv * pSHA256_Update)(SHA256_CTX *c, const void *data, size_t len);
    
    typedef int (CallConv * pSHA256_Final)(unsigned char *md, SHA256_CTX *c);
    
    typedef int (CallConv * pSHA512_Init)(SHA512_CTX *c);
    
    typedef int (CallConv * pSHA512_Update)(SHA512_CTX *c, const void *data, size_t len);
    
    typedef int (CallConv * pSHA512_Final)(unsigned char *md, SHA512_CTX *c);
    
    
    
	extern void ReleaseLibOpenSSL ();
	extern bool InitLibOpenSSL ();

	extern bool								openssl_LibInitialized;

    extern pOPENSSL_add_all_algorithms_noconf   dOPENSSL_add_all_algorithms_noconf;
    extern pOPENSSL_add_all_algorithms_conf     dOPENSSL_add_all_algorithms_conf;
    
    extern pERR_load_ERR_strings                dERR_load_ERR_strings;
    extern pERR_load_crypto_strings             dERR_load_crypto_strings;
    
    extern pBN_new                              dBN_new;
    extern pBN_set_word                         dBN_set_word;
    extern pRSA_new                             dRSA_new;
    extern pBN_free                             dBN_free;
    
    extern pEVP_sha1                            dEVP_sha1;
    extern pEVP_PKEY_new                        dEVP_PKEY_new;
    extern pEVP_PKEY_free                       dEVP_PKEY_free;
    extern pX509_free                           dX509_free;
    extern pRSA_generate_key                    dRSA_generate_key;
    extern pRSA_generate_key_ex                 dRSA_generate_key_ex;
    extern pEVP_PKEY_assign                     dEVP_PKEY_assign;
    extern pRSA_free                            dRSA_free;
    
    extern pASN1_INTEGER_set                    dASN1_INTEGER_set;
    extern pX509_gmtime_adj                     dX509_gmtime_adj;
    
    extern pX509_new                            dX509_new;
    extern pX509_set_pubkey                     dX509_set_pubkey;
    extern pX509_get_subject_name               dX509_get_subject_name;
    extern pX509_NAME_add_entry_by_txt          dX509_NAME_add_entry_by_txt;
    extern pX509_set_issuer_name                dX509_set_issuer_name;
    extern pX509_get_serialNumber               dX509_get_serialNumber;
    
    extern pX509_sign                           dX509_sign;
    extern pd2i_RSAPrivateKey                   dd2i_RSAPrivateKey;
    extern pi2d_RSAPrivateKey                   di2d_RSAPrivateKey;
    
    extern pd2i_X509                            dd2i_X509;
    extern pi2d_X509                            di2d_X509;
    extern pX509_get_pubkey                     dX509_get_pubkey;
    extern pEVP_PKEY_get1_RSA                   dEVP_PKEY_get1_RSA;
    extern pRSA_size                            dRSA_size;
    extern pRSA_public_encrypt                  dRSA_public_encrypt;
    extern pRSA_private_decrypt                 dRSA_private_decrypt;
    extern pERR_print_errors_fp                 dERR_print_errors_fp;
    
    extern pEVP_CIPHER_CTX_cleanup              dEVP_CIPHER_CTX_cleanup;
    extern pEVP_CIPHER_CTX_init                 dEVP_CIPHER_CTX_init;
    
    extern pEVP_PKEY_set1_RSA                   dEVP_PKEY_set1_RSA;
    extern pd2i_RSA_PUBKEY                      dd2i_RSA_PUBKEY;
    extern pPEM_read_X509                       dPEM_read_X509;
    extern pPEM_read_RSAPrivateKey              dPEM_read_RSAPrivateKey;
    
    extern pEVP_EncryptInit_ex                  dEVP_EncryptInit_ex;
    extern pEVP_DecryptInit_ex                  dEVP_DecryptInit_ex;
    extern pEVP_DecryptUpdate                   dEVP_DecryptUpdate;
    extern pEVP_DecryptFinal_ex                 dEVP_DecryptFinal_ex;
    extern pEVP_EncryptUpdate                   dEVP_EncryptUpdate;
    extern pEVP_EncryptFinal_ex                 dEVP_EncryptFinal_ex;
    extern pEVP_aes_256_cbc                     dEVP_aes_256_cbc;
    
    extern pSHA256_Init                         dSHA256_Init;
    extern pSHA256_Update                       dSHA256_Update;
    extern pSHA256_Final                        dSHA256_Final;
    
    extern pSHA512_Init                         dSHA512_Init;
    extern pSHA512_Update                       dSHA512_Update;
    extern pSHA512_Final                        dSHA512_Final;

} // -> namespace environs


#endif	/// INCLUDE_HCM_ENVIRONS_DYNAMIC_OPENSSL_H
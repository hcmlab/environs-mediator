/**
 * Environs Crypto Common Utils and Helpers
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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
//#   define DEBUGCIPHERS
#endif

#include "Environs.Crypt.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "Environs.Utils.h"
#include "Device.Info.h"
#include "Interop.h"

#ifndef ANDROID1
#   ifdef USE_OPENSSL
#       include "DynLib/Dyn.Lib.Crypto.h"
#   endif

#   ifdef _WIN32
#       include <Wincrypt.h>
#       include <Strsafe.h>

#       pragma comment ( lib, "Crypt32.lib" )
#       define USE_CACHED_HKEY
#   endif
#endif

#if !defined(ANDROID)
#   define ENABLE_DUMMY_CRYPT
#endif

#if ( defined(ENVIRONS_IOS) )
//#   define	ENABLE_CRYPT_EXCLUSIVE_PRIVKEY_ACCESS
#endif

#define CLASS_NAME	"Environs.Crypt . . . . ."

#ifdef _WIN32
#   pragma warning( push )
#   pragma warning( disable: 4996 )
#   pragma warning( push )
#   pragma warning( disable: 4995 )
#endif


namespace environs
{
	/// Some salt to the hash soup...
	const char *            hashSalt                = "hcmEnvirons";

#ifdef ENABLE_CRYPT_PRIVKEY_LOCKED_ACCESS
	/// A mutex to synchronize access to the private key
	pthread_mutex_t         privKeyMutex;
#endif
    
    bool                    allocated               = false;
    bool                    openssl_alg_added       = false;
    
    pEncryptMessage         EncryptMessage  = 0;
    pDecryptMessage         DecryptMessage  = 0;
    pReleaseCert            ReleaseCert     = 0;
    pSHAHashCreate          SHAHashCreate   = 0;
    pAESEncrypt             AESEncrypt      = 0;
    pAESDecrypt             AESDecrypt      = 0;
    
    pGenerateCertificate    GenerateCertificate     = 0;
    
    pAESDeriveKeyContext    AESDeriveKeyContext     = 0;
    pAESUpdateKeyContext    AESUpdateKeyContext     = 0;
    pAESDisposeKeyContext   AESDisposeKeyContext    = 0;
    pPreparePrivateKey      PreparePrivateKey       = 0;
    pDisposePublicKey       DisposePublicKey        = 0;
    pDisposePrivateKey      DisposePrivateKey       = 0;

	
	const char * ConvertToHexString ( const char * src, unsigned int length )
	{
		if ( !src || !length )
			return 0;

		static char buffer [4096];

		for ( unsigned int i = 0; i < length; i++ )
		{
			sprintf ( buffer + (i * 2), "%02X", (unsigned char)src [i] );
			//	CLogArg ( "%02x", (unsigned char)blob [i] );
		}

		return buffer;
	}
    
	const char * ConvertToHexSpaceString ( const char * src, unsigned int length )
	{
		if ( !src || !length )
			return 0;
        
		static char buffer [4096];
        
		for ( unsigned int i = 0; i < length; i++ )
		{
			sprintf ( buffer + (i * 3), "%02X ", (unsigned char)src [i] );
			//	CLogArg ( "%02x", (unsigned char)blob [i] );
		}
        
		return buffer;
    }
    
    const char * ConvertToHexSpaceBuffer ( const char * src, unsigned int length, char * buffer, bool limit )
    {
        if ( !src || !length )
            return 0;
        
		if ( limit && length > 300 )
			length = 300;
        for ( unsigned int i = 0; i < length; i++ )
        {
            sprintf ( buffer + (i * 3), "%02X ", (unsigned char)src [i] );
            //	CLogArg ( "%02x", (unsigned char)blob [i] );
        }
        
        buffer [ length * 3 ] = 0;
        
        return buffer;
    }


	char * ConvertToByteBuffer ( const char * src, unsigned int length, char * buffer )
	{
		length >>= 1;
		for ( unsigned int i = 0; i < length; i++ )
		{
			if ( sscanf_s ( src + (i * 2), "%2hhx", (unsigned char *)(buffer + i) ) != 1 )
			//if ( sscanf_s ( src + (i * 2), "%02X", (unsigned char *)(buffer + i) ) != 1 )
				return 0;
		}

		buffer [ length ] = 0;

		return buffer;
	}


	const char * ConvertToBytes ( const char * src, unsigned int length )
	{
		if ( !src || !length )
			return 0;

		static char buffer [1024];

		return ConvertToByteBuffer ( src, length, buffer );
	}


	char * ConvertToNewBytes ( const char * src, unsigned int length )
	{
		if ( !src || !length )
			return 0;

		char * buffer = (char *)calloc ( 1, length >> 1 );
		if ( !buffer )
			return 0;

		char * bufferNew = ConvertToByteBuffer ( src, length, buffer );
		if ( !bufferNew ) {
			free ( buffer );
		}
		return bufferNew;
	}
    
    
#ifdef USE_OPENSSL_AES
    
    /*
     * Additional functions for dynamic locks
     */
    struct CRYPTO_dynlock_value {
        pthread_mutex_t lock;
    };
    
    struct CRYPTO_dynlock_value * CryptDynLockCreate ( const char * file, int line )
    {
        CVerbVerbArg ( "CryptDynLockCreate: line [%i] file [%s]", line, file );
        
        struct CRYPTO_dynlock_value * value;
        
        value = (struct CRYPTO_dynlock_value *) malloc ( sizeof(struct CRYPTO_dynlock_value) );
        if ( !value ) {
            return 0;
        }
        
        if ( !MutexInitA ( value->lock ) ) {
            free ( value );
            return 0;
        }
        
        return value;
    }
    
    void CryptDynLockDispose ( struct CRYPTO_dynlock_value * l, const char * file, int line )
    {
        CVerbVerbArg ( "CryptDynLockDispose: line [%i] file [%s]", line, file );
        
        if ( !l )
            return;
        
        MutexDisposeA ( l->lock );
        
        free ( l );
    }
    
    
    unsigned long CryptoGetThreadID ( void )
    {
        unsigned long threadID;
        
        threadID = (unsigned long) GetCurrentThreadId (); // pthread_self();
        
        return threadID;
    }
    
    
    void CryptDynLockLock ( int mode, struct CRYPTO_dynlock_value * lock, const char *file, int line )
    {
        CVerbVerbArg ( "CryptDynLockLock: mode [%i] lock [%b] line [%i] file [%s]", mode, lock, line, file );
        
        if ( mode & CRYPTO_LOCK ) {
            pthread_mutex_lock ( &lock->lock );
        } else {
            pthread_mutex_unlock ( &lock->lock );
        }
    }
    
    // Static locks used by crypt layer
    pthread_mutex_t *    cryptLocks = 0;
    
    /*
     * Required for muti-threading
     */
    void CryptLock ( int mode, int n, const char * file, int line )
    {
        CVerbVerbArg ( "CryptLock: mode [%i] n [%i] line [%i] file [%s]", mode, n, line, file );
        
        if ( mode & CRYPTO_LOCK ) {
            pthread_mutex_lock ( &cryptLocks[n] );
        } else {
            pthread_mutex_unlock ( &cryptLocks[n] );
        }
    }
    
    
    bool CryptLocksCreate ()
    {
        CVerb ( "CryptLocksCreate" );
        
        cryptLocks = (pthread_mutex_t *) malloc ( dCRYPTO_num_locks () * sizeof(pthread_mutex_t) );
        if ( !cryptLocks ) {
            return false;
        }
        
        for ( int i = 0; i < dCRYPTO_num_locks (); i++ )
        {
            if ( !MutexInitA ( cryptLocks [ i ] ) )
                return false;
        }
        
        dCRYPTO_set_locking_callback            ( CryptLock );
        dCRYPTO_set_id_callback                 ( CryptoGetThreadID );
        
        dCRYPTO_set_dynlock_create_callback     ( CryptDynLockCreate );
        dCRYPTO_set_dynlock_destroy_callback    ( CryptDynLockDispose );
        dCRYPTO_set_dynlock_lock_callback       ( CryptDynLockLock );
        
        CVerbVerb ( "CryptLocksCreate: ok" );
        return true;
    }
    
    
    void CryptLocksDispose ()
    {
        CVerb ( "CryptLocksDispose" );
        
        if ( !cryptLocks )
            return;
        
        for ( int i = 0; i < dCRYPTO_num_locks (); i++ )
        {
            MutexDisposeA ( cryptLocks [ i ] );
        }
        
        free ( cryptLocks );
        cryptLocks = 0;
        
        dCRYPTO_set_locking_callback            ( 0 );
        dCRYPTO_set_id_callback                 ( 0 );
        
        dCRYPTO_set_dynlock_create_callback     ( 0 );
        dCRYPTO_set_dynlock_destroy_callback    ( 0 );
        dCRYPTO_set_dynlock_lock_callback       ( 0 );
        
        CVerbVerb ( "CryptLocksDispose: ok" );
    }
    
#else
    
#   define CryptLocksCreate()     true
#   define CryptLocksDispose()
#   define ReleaseLibOpenSSL()
    
#endif
    
    
#if (!defined(ANDROID))
    void dAESUpdateKeyContext ( AESContext * ctx, int deviceID )
    {
    }
    
#ifndef ENVIRONS_IOS
    bool UpdateKeyAndCert ( char * priv, char * cert )
    {        
        return true;
    }
#endif
#endif
    
#ifdef USE_OPENSSL
    void cryptAESUpdateKeyContext ( AESContext * ctx, int deviceID )
    {
    }
    
    bool cryptPreparePrivateKey ( char ** privKey )
    {
        if ( !privKey || !*privKey ) {
            CErr ( "PreparePrivateKey: Called with NULL argument" ); return false;
        }
        
        CVerb ( "PreparePrivateKey" );
        
        bool ret = false;
        
        RSA * rsaKey = 0;
        
        char * key = *privKey;
        *privKey = 0;
        
        const unsigned char * inKey = (const unsigned char *) (key + 4);
        unsigned int len = *((unsigned int *)key);
        if ( !len ) {
            CErr ( "PreparePrivateKey: Invalid length" ); goto Finish;
        }
        
        if ( !dd2i_RSAPrivateKey ( &rsaKey, &inKey, len) || !rsaKey ) {
            CErr ( "PreparePrivateKey: Converting key failed." ); goto Finish;
        }
        
        *privKey = (char *) rsaKey;
        rsaKey = 0;
        ret = true;
        
    Finish:
        if ( key )
            free ( key );
        if ( rsaKey )
            dRSA_free ( rsaKey );
        
        return ret;
    }
#endif
    
    bool dPreparePrivateKey ( char ** privKey )
    {
        return true;
    }

    
#if defined(USE_OPENSSL) || defined(_WIN32)
	
#ifdef _WIN32
	// AES 256-bit blob
	struct AES_256_BLOB {
		BLOBHEADER hdr;
		DWORD keySize;
		BYTE raw [32];
	};

	static const LPWSTR ENVIRONS_KEY_STORE_NAME = L"Environs";

	
	const LPWSTR GetUserEnvironsName ( unsigned int counter )
	{
		static WCHAR	userName [ 256 ];
		DWORD			size = 256;

		if ( !GetUserName ( userName, &size ) ) {
			CErr ( "GetUserEnvironsName: GetUserName Failed." );
			return 0;
		}

		static WCHAR	userKeyStoreName [ 1024 ];
		DWORD			KSsize = 1024;

		HRESULT hr = StringCbPrintfW ( userKeyStoreName, KSsize, L"%s.%u.%s", ENVIRONS_KEY_STORE_NAME, counter, userName );
		if ( hr != S_OK ){
			CErr ( "GetUserEnvironsName: StringCbPrintfW Failed." );
			return 0;
		}

		return userKeyStoreName;
	}


	bool AcquireContext ( HCRYPTPROV * hCSP, unsigned int counter, LPWSTR &keyStore )
	{
		keyStore = ENVIRONS_KEY_STORE_NAME;
		if ( counter > 0 )
			keyStore = GetUserEnvironsName ( counter );

		if ( !keyStore )
			return false;

		if ( !CryptAcquireContext ( hCSP, keyStore, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET ) ) { //CRYPT_VERIFYCONTEXT  | CRYPT_NEWKEYSET CRYPT_MACHINE_KEYSET | 

			CVerb ( "AcquireContext: CryptAcquireContext failed. We're gonna remove the container and try again..." );
			CryptAcquireContext ( hCSP, keyStore, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET | CRYPT_DELETEKEYSET );
				
			if ( !CryptAcquireContext ( hCSP, keyStore, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET ) ) { //CRYPT_VERIFYCONTEXT  | CRYPT_NEWKEYSET
				CErr ( "AcquireContext: CryptAcquireContext failed." );
				return false;
			}
		}

		return true;
	}
#endif
    
#  define dEVP_PKEY_assign_RSA(pkey,rsa) dEVP_PKEY_assign((pkey),EVP_PKEY_RSA,\
(char *)(rsa))
    
#ifdef MEDIATORDAEMON
    
	bool SignCertificate ( char * key, unsigned int keySize, char * pub, char ** cert )
	{
		bool ret = false;
        
#ifdef USE_OPENSSL
        RSA         *   rsa         = (RSA *) key;
        RSA         *   rsaPub      = 0;
        
        EVP_PKEY    *   pKey        = 0;
        EVP_PKEY    *   pPubKey     = 0;
        X509        *   x509        = 0;
        unsigned int    certDataSize = 0;
        char        *   certData    = 0;
        const unsigned char * certIn = (const unsigned char *) (pub + 4);
        
		do
		{
            pKey = dEVP_PKEY_new ();
            if ( !pKey ) {
                break;
            }
            
            if ( !dEVP_PKEY_set1_RSA ( pKey, rsa ) ) {
                break;
            }
            
            unsigned int certSize = *((unsigned int *)pub) & 0xFFFF;
			x509 = dd2i_X509 ( 0, &certIn, certSize );
            if ( !x509 ) {
                CVerb ( "SignCertificate: Read X509 cert file failed." );
                dERR_print_errors_fp ( stderr );
                
                if ( !dd2i_RSA_PUBKEY ( &rsaPub, &certIn, certSize ) ) {
                    CErr ( "SignCertificate: Read cert file and RSA public key failed." );
                    dERR_print_errors_fp ( stderr );
                    break;
                }
                CVerb ( "SignCertificate: Read RSA public key ok." );
                pPubKey = dEVP_PKEY_new ();
                if ( !pPubKey ) {
                    break;
                }
                
                if ( !dEVP_PKEY_assign_RSA ( pPubKey, rsaPub ) ) {
                    break;
                }
                
                x509 = dX509_new ();
                if ( !x509 ) {
                    break;
                }
                
                dX509_set_pubkey ( x509, pPubKey );
            }
            
            if ( !x509 ) {
                CErr ( "SignCertificate: Read cert data failed." );
                dERR_print_errors_fp ( stderr );
                break;
            }
            
            dASN1_INTEGER_set ( dX509_get_serialNumber ( x509 ), rand () );
            
            dX509_gmtime_adj ( X509_get_notBefore(x509), 0 );
            dX509_gmtime_adj ( X509_get_notAfter(x509), 94608000L );
            
            
            X509_NAME * subject = dX509_get_subject_name ( x509 );
            
            dX509_NAME_add_entry_by_txt ( subject, "C",  MBSTRING_ASC, (unsigned char *)"CA",        -1, -1, 0 );
            dX509_NAME_add_entry_by_txt ( subject, "O",  MBSTRING_ASC, (unsigned char *)"Environs", -1, -1, 0 );
            dX509_NAME_add_entry_by_txt ( subject, "CN", MBSTRING_ASC, (unsigned char *)"hcm-lab.de", -1, -1, 0 );
            
            dX509_set_issuer_name ( x509, subject );
            
            if ( !dX509_sign ( x509, pKey, dEVP_sha1 () ) ) { /// We should switch to sha2
//            if ( !X509_sign ( x509, pKey, EVP_sha () ) ) {
                break;
            }
            
            certDataSize = di2d_X509 ( x509, 0 );
            if ( certDataSize <= 0 ) {
                CErrArg ( "SignCertificate: i2d_X509 returned size [%i].", certDataSize ); break;
            }
            
            certData = (char *) malloc ( certDataSize + 4 );
            if ( !certData ) {
                CErrArg ( "SignCertificate: Memory allocation failed. size [%i].", certDataSize ); break;
            }
            
            unsigned char * certDataStore = (unsigned char *) (certData + 4);
            if ( !di2d_X509 ( x509, &certDataStore ) ) {
                CErr ( "SignCertificate: i2d_X509 failed." ); break;
            }
            *((unsigned int *) certData) = ('d' << 16 | certDataSize | ENVIRONS_CERT_SIGNED_FLAG);
            *cert = (char *) certData;
            certData = 0;
            
            ret = true;
		}
		while ( 0 );
        
        if ( x509 )     dX509_free ( x509 );
        if ( pKey )     dEVP_PKEY_free ( pKey );
        if ( certData ) free ( certData );
        
        if ( !ret ) {
            if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
            else
                if ( dERR_remove_state ) dERR_remove_state ( 0 );
        }
#else
#ifdef _WIN32
		bool						imported	= false;
		HCRYPTPROV					hCSP		= 0;
		HCRYPTKEY					hKey		= 0;
		CERT_PUBLIC_KEY_INFO	*	certPubInfo = 0;
		CERT_INFO					certInfo;
		DWORD						certInfoLen;
		LPWSTR						keyStore	= 0;
		char					*   certData    = 0;
		const char 				*	issuerStr		= "CN=Environs, T=Secure";
		const char 				*	subjStr			= "CN=Environs, T=Secure, Client";
		CRYPT_ALGORITHM_IDENTIFIER	alg;
		
		DWORD						nameLen		= 0;
		BYTE					*	nameSubject	= 0;
		BYTE					*	nameIssuer	= 0;
		BYTE					*	certBin		= (BYTE *) (pub + 4);
        unsigned int				certSize	= *((unsigned int *)pub) & 0xFFFF;

		do
		{            
			DWORD certBufferSize = 0;
			if ( !CryptStringToBinaryA ( (char *)certBin, certSize, CRYPT_STRING_ANY, 0, &certBufferSize, NULL, NULL ) || !certBufferSize ) {
				CErr ( "SignCertificate: CryptStringToBinaryA (retrieve size) failed." );
			}
			else {
				BYTE * certBuffer = (BYTE *) malloc ( certBufferSize );
				if ( !certBuffer ) {
					CErrArg ( "SignCertificate: Memory alloc for certBuffer failed [%u].", certSize ); break;
				}

				if ( !CryptStringToBinaryA ( (char *)certBin, certSize, CRYPT_STRING_ANY, certBuffer, &certBufferSize, NULL, NULL ) || !certBufferSize ) {
					CErr ( "SignCertificate: CryptStringToBinaryA (convert) failed." ); break;
				}
				certBin = certBuffer;
			}		

			certInfoLen = sizeof(CERT_PUBLIC_KEY_INFO);
			if ( CryptDecodeObjectEx ( X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, (BYTE *)certBin, certSize, CRYPT_ENCODE_ALLOC_FLAG, NULL, &certPubInfo, &certInfoLen ) ) {
				imported = true;
			}

			if ( !imported ) {
				certInfoLen = sizeof(CERT_INFO);
				if ( CryptDecodeObjectEx ( X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED, (BYTE *)certBin, certSize, CRYPT_DECODE_ALLOC_FLAG, NULL, &certInfo, &certInfoLen ) ) {
					imported = true;
				}
			}

			if ( !imported ) {
				break;
			}

			if ( !AcquireContext ( &hCSP, 0, keyStore ) ) {
				unsigned int trys = 3;
				do 
				{
					if ( AcquireContext ( &hCSP, trys, keyStore ) )
						break;
					trys--;
				} 
				while ( trys > 0 );
			}

			if ( !hCSP ) {
				CErr ( "SignCertificate: AcquireContext failed." ); break;
			}

			/*if ( !CryptImportKey ( hCSP, (BYTE *) key, keySize, NULL, 0, &hKey ) )
			{
				CErr ( "SignCertificate: CryptImportKey failed." ); break;
			}*/

			if ( !CryptGenKey ( hCSP, AT_SIGNATURE, CRYPT_EXPORTABLE, &hKey ) ) {
				CErr ( "SignCertificate: CryptGenKey failed." ); break;
			}

			Zero ( certInfo );

			certInfo.dwVersion = 2;

			BYTE serial [] = "\x01\x02\x04\x08\x0F\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
			certInfo.SerialNumber.cbData = 16;
			certInfo.SerialNumber.pbData = serial;

			certInfo.SignatureAlgorithm.pszObjId = szOID_RSA_SHA1RSA;
			certInfo.SignatureAlgorithm.Parameters.cbData = 0;

			CERT_RDN_ATTR nameAttr =
			{
				szOID_COMMON_NAME, 
				CERT_RDN_PRINTABLE_STRING, 
				(DWORD) (sizeof(issuerStr) + 1), 
				(BYTE *)issuerStr 
			};

			CERT_RDN rgRDN [] = { 1, &nameAttr };

			CERT_NAME_INFO CertName = {
				1, rgRDN
			};


			if ( !CryptEncodeObjectEx ( X509_ASN_ENCODING, X509_NAME, &CertName, 0, NULL, NULL, &nameLen ) || nameLen <= 0 ) {
				CErr ( "SignCertificate: AcquireContext failed." ); break;
			}

			nameIssuer = ( BYTE* ) malloc ( nameLen );
			if ( !nameIssuer ) {
				CErrArg ( "SignCertificate: Memory alloc for nameIssuer failed [%u].", certSize ); break;
			}

			if ( !CryptEncodeObjectEx ( X509_ASN_ENCODING, X509_NAME, &CertName, 0, NULL, nameIssuer, &nameLen ) )	 {
				CErr ( "SignCertificate: CryptEncodeObjectEx nameIssuer failed." ); break;
			}

			certInfo.Issuer.cbData = nameLen;
			certInfo.Issuer.pbData = nameIssuer;

			SYSTEMTIME SysTime;
			GetSystemTime ( &SysTime );
			SystemTimeToFileTime ( &SysTime, &certInfo.NotBefore );

			SysTime.wYear += 3;
			SystemTimeToFileTime ( &SysTime, &certInfo.NotAfter );

			nameAttr.pszObjId		= szOID_COMMON_NAME;
			nameAttr.dwValueType	= CERT_RDN_PRINTABLE_STRING;
			nameAttr.Value.cbData	= (DWORD) (sizeof(subjStr) + 1);
			nameAttr.Value.pbData	= (PBYTE) subjStr;

			if ( !CryptEncodeObjectEx ( X509_ASN_ENCODING, X509_NAME, &CertName, 0, NULL, NULL, &nameLen ) ) {
				CErr ( "SignCertificate: CryptEncodeObjectEx nameSubject len failed." ); break;
			}

			nameSubject = (BYTE*) malloc ( nameLen );
			if ( !nameSubject ) {
				CErrArg ( "SignCertificate: Memory alloc for nameSubject failed [%u].", certSize ); break;
			}

			if ( !CryptEncodeObjectEx ( X509_ASN_ENCODING, X509_NAME, &CertName, 0, NULL, nameSubject, &nameLen ) ) {
				CErr ( "SignCertificate: CryptEncodeObjectEx nameSubject failed." ); break;
			}

			certInfo.Subject.cbData = nameLen;
			certInfo.Subject.pbData = nameSubject;
			certInfo.SubjectPublicKeyInfo = *certPubInfo;

			certInfo.cExtension = 0;
			certInfo.rgExtension = NULL;

			certInfo.IssuerUniqueId.cbData = 0 ;
			certInfo.SubjectUniqueId.cbData = 0;

			BYTE paraData [16];
			paraData [0] = 0x05; paraData [1] = 0x00;

			alg.pszObjId = szOID_RSA_SHA1RSA;
			alg.Parameters.cbData = 2;
			alg.Parameters.pbData = paraData;

			if ( !CryptSignAndEncodeCertificate ( hCSP, AT_SIGNATURE, X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED, (void*)&certInfo, &alg, NULL, NULL, (DWORD *) &certSize ) ) {
				CErr ( "SignCertificate: CryptSignAndEncodeCertificate length failed." ); break;
			}

			certData = (char *)malloc ( certSize + 4 );
			if ( !certData ) {
				CErrArg ( "SignCertificate: Memory alloc for certData failed [%u].", certSize ); break;
			}

			if ( !CryptSignAndEncodeCertificate ( hCSP, AT_SIGNATURE, X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED, (void*)&certInfo, &alg, NULL, (PBYTE)(certData + 4), (DWORD *) &certSize ) ) {
				CErr ( "SignCertificate: CryptSignAndEncodeCertificate failed." ); break;
			}

			*((unsigned int *)certData) = ('d' << 16 | certSize | ENVIRONS_CERT_SIGNED_FLAG);
			*cert = (char *)certData;
			certData = 0;

			ret = true;
		} 
		while ( 0 );

		if ( !ret ) {
			CErrArg ( "SignCertificate: Last error [0x%08x]", GetLastError ( ) );
		}

		if ( certBin != (BYTE *)(cert + 4)) free ( certBin );
		if ( nameIssuer )	free ( nameIssuer );
		if ( nameSubject )	free ( nameSubject );
		if ( certData )		free ( certData );
		if ( certPubInfo )	LocalFree ( certPubInfo );
		if ( hKey )			CryptDestroyKey ( hKey );
		if ( hCSP )			CryptReleaseContext ( hCSP, 0 );

#endif
#endif
		return ret;
	}
#endif


#ifdef USE_OPENSSL

#ifdef ENABLE_DUMMY_CRYPT
    bool dGenerateCertificate ( char ** priv, char ** pub )
    {
        return false;
    }
#endif
    
	bool cryptGenerateCertificate ( char ** priv, char ** pub )
	{
		bool ret = false;
        
		char		*	privKey		= 0;
        EVP_PKEY    *   pKey        = 0;
        RSA         *   rsa         = 0;
        X509        *   x509        = 0;
        unsigned int    certDataSize = 0;
        char        *   certData    = 0;
        BIGNUM      *   bne         = 0;
        
        unsigned char * tmpPrivKey  = 0;
        
		do
		{
            pKey = dEVP_PKEY_new ();
            if ( !pKey ) {
                break;
            }
            
            if ( dRSA_generate_key ) {
                rsa = dRSA_generate_key ( ENVIRONS_DEVICES_KEYSIZE, RSA_F4, NULL, NULL );
            }
            else {
                bne = dBN_new ();
                int success = ret = dBN_set_word ( bne, RSA_F4 );
                if ( success != 1 ) {
                    break;
                }
                
                rsa = dRSA_new ();
                
                success = dRSA_generate_key_ex ( rsa, ENVIRONS_DEVICES_KEYSIZE, bne, NULL );
                if ( success != 1 ) {
                    break;
                }
            }
            if ( !dEVP_PKEY_assign_RSA ( pKey, rsa ) ) {
                break;
            }
            
            x509 = dX509_new ();
            if ( !x509 ) {
                break;
            }
            
            dASN1_INTEGER_set( dX509_get_serialNumber ( x509 ), rand () );
            
            dX509_gmtime_adj ( X509_get_notBefore(x509), 0 );
            dX509_gmtime_adj ( X509_get_notAfter(x509), 94608000L );
            
            dX509_set_pubkey ( x509, pKey );
            
            X509_NAME * subject = dX509_get_subject_name ( x509 );
            
            dX509_NAME_add_entry_by_txt ( subject, "C",  MBSTRING_ASC, (unsigned char *)"CA",        -1, -1, 0 );
            dX509_NAME_add_entry_by_txt ( subject, "O",  MBSTRING_ASC, (unsigned char *)"Environs", -1, -1, 0 );
            dX509_NAME_add_entry_by_txt ( subject, "CN", MBSTRING_ASC, (unsigned char *)"hcm-lab.de", -1, -1, 0 );
            
            dX509_set_issuer_name ( x509, subject );
            
            if ( !dX509_sign ( x509, pKey, dEVP_sha1 () ) ) {  /// We should switch to sha2
                break;
            }
            
            certDataSize = di2d_X509 ( x509, 0 );
            if ( certDataSize <= 0 ) {
                CErrArg ( "GenerateCertificate: i2d_X509 returned size [%i].", certDataSize ); break;
            }
            
            certData = (char *) calloc ( 1, certDataSize + 4 );
            if ( !certData ) {
                CErrArg ( "GenerateCertificate: Memory allocation failed. size [%i].", certDataSize ); break;
            }
            
            unsigned char * certDataStore = (unsigned char *) (certData + 4);
            if ( !di2d_X509 ( x509, &certDataStore ) ) {
                CErr ( "GenerateCertificate: i2d_X509 failed." ); break;
            }
            *((unsigned int *) certData) = ('d' << 16 | certDataSize | ENVIRONS_CERT_SIGNED_FLAG);
            *pub = (char *) certData;
            certData = 0;
            
            unsigned int privKeyLen = di2d_RSAPrivateKey(rsa, 0);
            if ( privKeyLen <= 0 ) {
                break;
            }
            certData = (char *) calloc ( 1, privKeyLen + 4 );
            if ( !certData ) {
                CErrArg ( "GenerateCertificate: Memory allocation failed. size [%i].", privKeyLen ); break;
            }
            
            privKeyLen = di2d_RSAPrivateKey (rsa, (unsigned char **)&tmpPrivKey);
            
            if ( privKeyLen <= 0 || !tmpPrivKey ) {
                CErr ( "GenerateCertificate: Failed to get private key." );
                break;
            }
            memcpy ( certData + 4, tmpPrivKey, privKeyLen );
            
            *((unsigned int *) certData) = privKeyLen;
            *priv = (char *) certData;
            certData = 0;
            
            
            ret = true;
		}
		while ( 0 );
        
        if ( bne )      dBN_free ( bne );
        if ( x509 )     dX509_free ( x509 );
        if ( pKey )     dEVP_PKEY_free ( pKey );
//        if ( rsa )      dRSA_free ( rsa );
		if ( privKey )	free ( privKey );
        if ( certData ) free ( certData );
		return ret;
	}
#else
    
#ifdef _WIN32
    

	bool cryptGenerateCertificate ( char ** priv, char ** pub )
	{
		bool ret = false;

		HCRYPTPROV		hCSP		= 0;
		HCRYPTKEY		hKey		= 0;
		char		*	privKey		= 0;
		char		*	pubKey		= 0;
		BYTE		*	certSubj	= 0;
		DWORD			certSubjLen;
		DWORD			length;
		PCCERT_CONTEXT	certContext = 0;
		LPCTSTR			certStr		= L"CN=Environs, T=Secure";

		CERT_NAME_BLOB				certName;
		CRYPT_KEY_PROV_INFO			keyProv;
		CRYPT_ALGORITHM_IDENTIFIER	alg;
		LPWSTR			keyStore	= 0;

		do
		{
			if ( !AcquireContext ( &hCSP, 0, keyStore ) ) {
				unsigned int trys = 3;
				do 
				{
					if ( AcquireContext ( &hCSP, trys, keyStore ) )
						break;
					trys--;
				} 
				while ( trys > 0 );
			}

			if ( !hCSP ) {
				CErr ( "GenerateCertificate: AcquireContext failed." ); break;
			}

			if ( !CryptGenKey ( hCSP, CALG_RSA_KEYX, (ENVIRONS_DEVICES_KEYSIZE << 16) | CRYPT_EXPORTABLE, &hKey ) ) {//0x08000000 // AT_KEYEXCHANGE
				CErr ( "GenerateCertificate: CryptGenKey failed." ); break;
			}

			certSubjLen = 0;
			if ( !CertStrToName ( X509_ASN_ENCODING, certStr, CERT_X500_NAME_STR, NULL, 0, &certSubjLen, NULL ) ) {
				CErr ( "GenerateCertificate: CertStrToName length failed." ); break;
			}

			certSubj = (BYTE *)malloc ( certSubjLen );
			if ( !certSubj ) {
				CErr ( "GenerateCertificate: Memory alloc for pubKeyInfo failed." ); break;
			}

			if ( !CertStrToName ( X509_ASN_ENCODING, certStr, CERT_X500_NAME_STR, NULL, certSubj, &certSubjLen, NULL ) ) {
				CErr ( "GenerateCertificate: CertStrToName failed." ); break;
			}

			Zero ( certName );
			certName.cbData = certSubjLen;
			certName.pbData = certSubj;

			Zero ( keyProv );
			keyProv.pwszContainerName	= keyStore;
			keyProv.pwszProvName		= 0;
			keyProv.dwProvType			= PROV_RSA_FULL;
			keyProv.dwFlags				= (ENVIRONS_DEVICES_KEYSIZE << 16) | CERT_SET_KEY_CONTEXT_PROP_ID; // CRYPT_MACHINE_KEYSET;
			keyProv.cProvParam			= 0;
			keyProv.rgProvParam			= NULL;
			keyProv.dwKeySpec			= AT_KEYEXCHANGE;

			Zero ( alg );
			alg.pszObjId = szOID_RSA_SHA1RSA;  /// We should switch to sha2

			SYSTEMTIME validTime;
			GetSystemTime ( &validTime );
			validTime.wYear += 3;

			certContext = CertCreateSelfSignCertificate ( hCSP, &certName, (ENVIRONS_DEVICES_KEYSIZE << 16), &keyProv, &alg, 0, &validTime, 0 );
			if ( !certContext ) {
				CErr ( "GenerateCertificate: CertCreateSelfSignCertificate length failed." ); break;
			}

			length = certContext->cbCertEncoded;
			pubKey = (char *)malloc ( length + 4 );
			if ( !pubKey ) {
				CErr ( "GenerateCertificate: Memory alloc for pubKey failed." ); break;
			}

			memcpy ( pubKey + 4, certContext->pbCertEncoded, length );

			*((unsigned int *)pubKey) = (('d' << 16) | length | ENVIRONS_CERT_SIGNED_FLAG);
			
			length = 0;
			if ( !CryptExportKey ( hKey, 0, PRIVATEKEYBLOB, 0, NULL, &length ) ) {
				CErr ( "GenerateCertificate: CryptExportKey PRIVATEKEYBLOB length failed." ); break;
			}

			privKey = (char *) malloc ( length + 4 );
			if ( !privKey ) {
				CErr ( "GenerateCertificate: Memory alloc for privKey failed." ); break;
			}
			if ( !CryptExportKey ( hKey, NULL, PRIVATEKEYBLOB, 0, (PBYTE)(privKey + 4), &length ) ) {
				CErr ( "GenerateCertificate: CryptExportKey PRIVATEKEYBLOB failed." ); break;
			}
			*((unsigned int *)privKey) = length;

			*priv = privKey;
			privKey = 0;

			*pub = pubKey;
			pubKey = 0;
			ret = true;
		} 
		while ( 0 );
		
		if ( !ret ) {
			DWORD err = GetLastError ( );
			CErrArg ( "GenerateCertificate: Last error [0x%08x] / [%u]", err, err );
		}

		if ( certSubj )	free ( certSubj );
		if ( privKey )	free ( privKey );
		if ( pubKey )	free ( pubKey );
		if ( certContext )	CertFreeCertificateContext ( certContext );
		if ( hKey )		CryptDestroyKey ( hKey );
		if ( hCSP )		CryptReleaseContext ( hCSP, 0 );

		return ret;
	}
#endif
#endif
    
#ifdef ENABLE_DUMMY_CRYPT
    bool dEncryptMessage ( int deviceID, char * cert, char * msg, unsigned int * msgLen )
    {
        return false;
    }
#endif
    
	bool cryptEncryptMessage ( int deviceID, char * cert, char * msg, unsigned int * msgLen )
	{
		if ( !cert || !msg || !msgLen ) {
			CErrID ( "EncryptMessage: Called with at least one null argument." );
			return false;
		}

		unsigned int certProp = *((unsigned int *)cert);
		unsigned int certSize = certProp & 0xFFFF;
        
        /// Determine format of certificate
		unsigned int format = (certProp >> 16) & 0xFF;
        
		CVerbArgID ( "EncryptMessage: Encrypting buffer sized [%i]", *msgLen );

//		CVerbArgID ( "EncryptMessage: Cert size [%u] [%s]", certSize, ConvertToHexSpaceString ( cert + 4, certSize ) );
//
//#ifdef DEBUGVERB
//		char c = cert [certSize]; cert [certSize] = 0;
//		CVerbArg ( "EncryptMessage: Cert raw [%s]", cert + 4 );
//		cert [certSize] = c;
//#endif
		bool ret = false;

#ifdef USE_OPENSSL
        if ( !openssl_alg_added ) {
			if ( dOpenSSL_add_all_algorithms )
				dOpenSSL_add_all_algorithms ();
			else
# ifdef OPENSSL_LOAD_CONF
				if ( dOPENSSL_add_all_algorithms_conf )
					dOPENSSL_add_all_algorithms_conf();
# else
				if ( dOPENSSL_add_all_algorithms_noconf )
					dOPENSSL_add_all_algorithms_noconf();
# endif
            dERR_load_crypto_strings();
            
            openssl_alg_added = true;
        }

        EVP_PKEY    *   pkey	= 0;
        X509		*	cert509 = 0;
        RSA			*	rsaKey	= 0;
		char		*	ciphers = 0;
		unsigned int	cipherSize = 0;
        const unsigned char * certD = (unsigned char *) (cert + 4);
        
        do
        {            
			cert509 = dd2i_X509 ( 0, &certD, certSize );
            if ( !cert509 ) {
				CErrArgID ( "EncryptMessage: Read X509 cert file (format %c) failed.", format );
                dERR_print_errors_fp ( stderr );
                break;
            }
            
            pkey = dX509_get_pubkey ( cert509 );
            if ( !pkey ) {
				CErrID ( "EncryptMessage: Failed to load public key from certificate." ); break;
            }
            
            rsaKey = dEVP_PKEY_get1_RSA ( pkey );
            if ( !rsaKey ) {
				CErrID ( "EncryptMessage: Failed to load public key from certificate." ); break;
            }

            cipherSize = (unsigned int) dRSA_size(rsaKey);
            //unsigned char * pubK = 0;
            //unsigned int pubKSize =
            //i2d_RSAPublicKey(rsaKey, &pubK);
			/// Print the public key blob
			//CLogArg ( "EncryptMessage: Public key [%s]", ConvertToHexSpaceString ( (const char *)pubK, pubKSize ) );
            
            ciphers = (char *) malloc ( cipherSize );
            if ( !ciphers ) {
				CErrID ( "EncryptMessage: Memory alloc failed." ); break;
            }
            
			int pad = 0;
			//if ( certProp & ENVIRONS_CRYPT_PAD_OAEP )
			//	pad = RSA_PKCS1_OAEP_PADDING;
			if ( certProp & ENVIRONS_CRYPT_PAD_PKCS1 )
                pad = RSA_PKCS1_PADDING;
            else
                pad = RSA_PKCS1_OAEP_PADDING; //RSA_NO_PADDING;

            int ciphersSize = dRSA_public_encrypt ( *msgLen, (unsigned char *)msg, (unsigned char *)ciphers, rsaKey, pad );
            if ( ciphersSize <= 0 ) {
				CErrID ( "EncryptMessage: Encrypt failed." ); break;
            }
            memcpy ( msg, ciphers, cipherSize );
            *msgLen = cipherSize;
            ret = true;
        }
        while ( 0 );
        
		if ( ciphers )	free ( ciphers );
        if ( pkey )		dEVP_PKEY_free ( pkey );
        if ( cert509 )	dX509_free ( cert509 );
        if ( rsaKey )	dRSA_free ( rsaKey );
        
        if ( !ret ) {
            if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
            else
                if ( dERR_remove_state ) dERR_remove_state ( 0 );
        }
#else
		

#ifdef _WIN32
		bool						imported	= false;
		HCRYPTPROV					hCSP		= 0;
		HCRYPTKEY					hKey		= 0;
		CERT_PUBLIC_KEY_INFO	*	certPubInfo = 0;
		CERT_INFO				*	certInfo	= 0;
		DWORD						certInfoLen;
		//char					*	ciphers		= 0;
		char					*	reverseBuffer = 0;
		BYTE					*	certBin		= (BYTE *) (cert + 4);

		do
		{
			if ( !CryptAcquireContext ( &hCSP, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) ) {
				CErrID ( "EncryptMessage: CryptAcquireContext failed." ); break;
			}
            
			if ( format == 'p' ) {
				DWORD certBufferSize = 0;
				if ( !CryptStringToBinaryA ( (char *)certBin, certSize, CRYPT_STRING_ANY, 0, &certBufferSize, NULL, NULL ) || !certBufferSize )
				{
					CErrID ( "EncryptMessage: CryptStringToBinaryA (retrieve size) failed." ); break;
				}

				BYTE * certBuffer = (BYTE *) malloc ( certBufferSize );
				if ( !certBuffer ) {
					CErrArgID ( "EncryptMessage: Memory alloc failed [%u].", certBufferSize ); break;
				}

				if ( !CryptStringToBinaryA ( (char *)certBin, certSize, CRYPT_STRING_ANY, certBuffer, &certBufferSize, NULL, NULL ) || !certBufferSize )
				{
					CErrID ( "EncryptMessage: CryptStringToBinaryA (convert) failed." ); break;
				}
				certBin = certBuffer;
			}			

			certInfoLen = sizeof(CERT_PUBLIC_KEY_INFO);
			if ( CryptDecodeObjectEx ( X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, (BYTE *)certBin, certSize, CRYPT_ENCODE_ALLOC_FLAG, NULL, &certPubInfo, &certInfoLen ) )
			{
				if ( !CryptImportPublicKeyInfo ( hCSP, X509_ASN_ENCODING, certPubInfo, &hKey ) ) {
					CErrID ( "EncryptMessage: CryptImportPublicKeyInfo (1) failed." );
				}
				else imported = true;
			}

			if ( !imported ) {
				certInfoLen = sizeof(CERT_INFO);
				if ( CryptDecodeObjectEx ( X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED, (BYTE *)certBin, certSize, CRYPT_DECODE_ALLOC_FLAG, NULL, &certInfo, &certInfoLen ) )
				{
					if ( !CryptImportPublicKeyInfo ( hCSP, X509_ASN_ENCODING, &certInfo->SubjectPublicKeyInfo, &hKey ) ) {
						CErrID ( "EncryptMessage: CryptImportPublicKeyInfo (2) failed." );
					}
					else imported = true;
				}
			}

			if ( !imported ) {
				if ( !CryptImportKey ( hCSP, (BYTE *)certBin, certSize, NULL, 0, &hKey ) ) {
					CErrID ( "EncryptMessage: CryptImportKey (3) failed." ); break;
				}
				else imported = true;
			}

            int pad = 0;
			//if ( certProp & ENVIRONS_CRYPT_PAD_OAEP )
                pad = CRYPT_OAEP;
            //else
			//	pad = 0;

			DWORD ciphersLen = (DWORD)*msgLen;

			DWORD reqSize = ciphersLen;
			if ( !CryptEncrypt ( hKey, 0, TRUE, pad, 0, &reqSize, reqSize ) || !reqSize )
			{
				CErrID ( "EncryptMessage: CryptEncrypt retrieve cipher text size failed ." ); break;
			}

			reverseBuffer = (char *) malloc ( reqSize );
			if ( !reverseBuffer ) {
				CErrID ( "EncryptMessage: Allocation of reverse buffer failed ." ); break;
			}
			memcpy ( reverseBuffer, msg, ciphersLen );

			if ( !CryptEncrypt ( hKey, 0, TRUE, pad, (BYTE *) reverseBuffer, &ciphersLen, reqSize ) )
			{
				CErrID ( "EncryptMessage: CryptEncrypt failed ." ); break;
			}

			for ( unsigned int i = 0; i < ciphersLen; ++i )
				msg [i] = reverseBuffer [ciphersLen - 1 - i];
			
			*msgLen = ciphersLen;
			ret = true;	

			CVerbArgID ( "EncryptMessage: Encrypted message size [%i]", ciphersLen );
		} 
		while ( 0 );

		if ( !ret ) {
			CErrArgID ( "EncryptMessage: Last error [0x%08x]", GetLastError ( ) );
		}

		if ( certBin != (BYTE *)(cert + 4)) free ( certBin );
		//if ( ciphers )		free ( ciphers );
		if ( reverseBuffer ) free ( reverseBuffer );
		if ( certInfo )		LocalFree ( certInfo );
		if ( certPubInfo )	LocalFree ( certPubInfo );
		if ( hKey )			CryptDestroyKey ( hKey );
		if ( hCSP )			CryptReleaseContext ( hCSP, 0 );
#endif

#endif
		return ret;
	}

    
#ifdef ENABLE_DUMMY_CRYPT
    bool dDecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize )
    {
        return false;
    }
#endif
    
	bool cryptDecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize )
	{
		if ( !key || !msg || !msgLen || !decrypted || !decryptedSize ) {
			CErr ( "DecryptMessage: Called with at least one null argument." );
			return false;
        }
		CVerbArg ( "DecryptMessage: Decrypting msg of size [%i]", msgLen );

		bool success = false;

#ifdef ENABLE_CRYPT_EXCLUSIVE_PRIVKEY_ACCESS
		if ( pthread_mutex_lock ( &privKeyMutex ) ) {
			CErr ( "DecryptMessage: Failed to acquire lock." );
			return false;
		}
#endif

#ifdef USE_OPENSSL
        RSA     * rsa       = (RSA *) key;
        char    * decrypt   = 0;
		unsigned int rsaSize = (unsigned int) dRSA_size(rsa);
        
        do
        {
            if ( msgLen != rsaSize ) {
				CErrArg ( "DecryptMessage: msgLen [%u] must be [%u].", msgLen, rsaSize ); break;
            }
            
            decrypt = (char *) malloc ( rsaSize + 1 );
            if ( !decrypt ) {
				CErr ( "DecryptMessage: Failed to allocate memory." ); break;
            }
            
			CVerb ( "DecryptMessage: Decrypting..." );
            
			int decSize = -1;
            
            do
            {                
                decSize = dRSA_private_decrypt ( rsaSize, (unsigned char*)msg, (unsigned char*)decrypt, rsa, RSA_PKCS1_OAEP_PADDING );
                if ( decSize <= 0 ) {
                    dERR_print_errors_fp ( stderr );
					CWarnArg ( "DecryptMessage: RSA_PKCS1_OAEP_PADDING Decrypted message size is [%i].", decSize );
                }
                else break;

                decSize = dRSA_private_decrypt ( rsaSize, (unsigned char*)msg, (unsigned char*)decrypt, rsa, RSA_PKCS1_PADDING );
                if ( decSize <= 0 ) {
                    dERR_print_errors_fp ( stderr );
					CWarnArg ( "DecryptMessage: RSA_PKCS1_PADDING Decrypted message size is [%i].", decSize );
                }
                else break;

                decSize = dRSA_private_decrypt ( rsaSize, (unsigned char*)msg, (unsigned char*)decrypt, rsa, RSA_PKCS1_PSS_PADDING );
                if ( decSize <= 0 ) {
                    dERR_print_errors_fp ( stderr );
					CWarnArg ( "DecryptMessage: RSA_PKCS1_PSS_PADDING Decrypted message size is [%i].", decSize );
                }
                /*
                 else break;
                decSize = dRSA_private_decrypt ( rsaSize, (unsigned char*)msg, (unsigned char*)decrypt, rsa, RSA_NO_PADDING );
                if ( decSize <= 0 ) {
                    dERR_print_errors_fp ( stderr );
					CWarnArg ( "DecryptMessage: RSA_NO_PADDING Decrypted message size is [%i].", decSize );
                }
                */
            }
            while ( 0 );

            if ( decSize <= 0 ) {
                dERR_print_errors_fp ( stderr );
				CErrArg ( "DecryptMessage: All padding types failed. Decrypted message size is [%i].", decSize ); break;
			}

			if ( decSize > (int)rsaSize ) {
                dERR_print_errors_fp ( stderr );
				CErrArg ( "DecryptMessage: Decrypted message size [%u] is larger than RSA buffer.", decSize ); break;
			}

            decrypt [ decSize ] = 0;
			//CVerbVerbArg ( "DecryptMessage: [%s]", decrypt );
            
            *decrypted = (char *)decrypt;
            decrypt = 0;
            *decryptedSize = (unsigned int)decSize;
            success = true;
        }
        while (0);

		if ( decrypt )
			free ( decrypt );
        
        if ( !success ) {
            if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
            else
                if ( dERR_remove_state ) dERR_remove_state ( 0 );
        }
#else

#ifdef _WIN32
		HCRYPTPROV		hCSP		= 0;
		HCRYPTKEY		hKey		= 0;
		char		*	plainText	= 0;
		DWORD			plainSize	= 0;
        
#ifndef MEDIATORDAEMON
        key += 4;
#endif
		do
		{
			if ( !CryptAcquireContext ( &hCSP, NULL, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) ) 
			{
				CErr ( "DecryptMessage: CryptAcquireContext failed." ); break;
			}
			
			if ( !CryptImportKey ( hCSP, (BYTE *) key, keySize, NULL, 0, &hKey ) )
			{
				CErr ( "DecryptMessage: CryptImportKey failed." ); break;
			}

			plainText = (char *) malloc ( msgLen + 2 );
			if ( !plainText ) {
				CErrArg ( "DecryptMessage: Memory allocation [%i bytes] failed.", msgLen ); break;
			}

			for ( unsigned int i = 0; i < msgLen; i++ )
				plainText [i] = msg [msgLen - 1 - i];
            
			plainSize = msgLen;
			if ( CryptDecrypt ( hKey, NULL, TRUE, CRYPT_OAEP, (BYTE *) plainText, &plainSize ) && plainSize )
				success = true;
			else
			{
				CVerb ( "DecryptMessage: CryptDecrypt failed with OAEP padding." );

				plainSize = msgLen;
				if ( CryptDecrypt ( hKey, NULL, TRUE, 0, (BYTE *) plainText, &plainSize ) && plainSize )
					success = true;
				else
				{
					CVerb ( "DecryptMessage: CryptDecrypt failed without specific padding flags." );

					plainSize = msgLen;
					if ( CryptDecrypt ( hKey, NULL, TRUE, CRYPT_DECRYPT_RSA_NO_PADDING_CHECK, (BYTE *) plainText, &plainSize ) && plainSize )
						success = true;
					else {
						CErr ( "DecryptMessage: CryptDecrypt failed with no padding check." ); break;
					}
				}
			}
			
			if ( success ) {
				*decryptedSize = plainSize;
				plainText [plainSize] = 0;
				CVerbVerbArg ( "DecryptMessage: size [%u] ", plainSize );

				*decrypted = plainText;
				plainText = 0;
			}
		}
		while ( 0 );

		if ( !success ) {
			CErrArg ( "DecryptMessage: Last error [0x%08x]", GetLastError ( ) );
		}

		if ( plainText )	free ( plainText );
		if ( hKey )			CryptDestroyKey ( hKey );
		if ( hCSP )			CryptReleaseContext ( hCSP, 0 );
#endif

#endif

#ifdef ENABLE_CRYPT_EXCLUSIVE_PRIVKEY_ACCESS
		if ( pthread_mutex_unlock ( &privKeyMutex ) ) {
			CErr ( "DecryptMessage: Failed to release lock." );
			success = false;
		}
#endif
		return success;
	}
    
#ifdef ENABLE_DUMMY_CRYPT
    void dReleaseCert ( int deviceID )
    {
    }
#endif

    void cryptReleaseCert ( int deviceID )
    {
        
    }
    
#ifdef ENABLE_DUMMY_CRYPT
    void dAESDisposeKeyContext ( AESContext * ctx )
    {
    }
#endif
	
	void cryptAESDisposeKeyContext ( AESContext * ctx )
	{
		CVerb ( "AESDisposeKeyContext" );

		if ( !ctx || !ctx->encCtx )
			return;
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
        MutexDispose ( &ctx->encLock );
        MutexDispose ( &ctx->decLock );
#endif

#ifdef USE_OPENSSL_AES
		if ( ctx->encCtx ) {
			EVP_CIPHER_CTX * ect = (EVP_CIPHER_CTX *) ctx->encCtx;
			dEVP_CIPHER_CTX_cleanup ( ect );
			free ( ctx->encCtx );
		}
		if ( ctx->decCtx ) {
			EVP_CIPHER_CTX * ect = (EVP_CIPHER_CTX *) ctx->decCtx;
			dEVP_CIPHER_CTX_cleanup ( ect );
            free ( ctx->decCtx );
		}
#else
#   ifdef USE_CACHED_HKEY
		if ( ctx->encCtx ) CryptDestroyKey ( (HCRYPTKEY) ctx->encCtx );
		if ( ctx->keyCtx ) CryptReleaseContext ( (HCRYPTPROV) ctx->keyCtx, 0 );
#   else
		if ( ctx->encCtx ) free ( ctx->encCtx );
#   endif
#endif
		memset ( ctx, 0, sizeof(AESContext) );
	}
    
#ifdef ENABLE_DUMMY_CRYPT
    bool dAESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx )
    {
        return false;
    }
#endif

	bool cryptAESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx )
	{
		CVerb ( "AESDeriveKeyContexts" );

		if ( !key || keyLen < AES_SHA256_KEY_LENGTH || !ctx ) {
			CErrArg ( "AESDeriveKeyContexts: Called with at least one NULL argument or keyLen [%u] < [%u].", keyLen, AES_SHA256_KEY_LENGTH ); return false;
		}

		char		*	blob		= 0;
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
        if ( !MutexInit ( &ctx->encLock ) )
            return false;
        
        if ( !MutexInit ( &ctx->decLock ) )
            return false;
#endif

		bool ret = false;

#ifdef USE_OPENSSL_AES
		do
		{
			SHA256_CTX sha;
			Zero ( sha );

			blob = (char *) malloc ( AES_SHA256_KEY_LENGTH );
			if ( !blob ) {
				CErr ( "AESDeriveKeyContexts: Failed to allocate memory for hashed blob or IV." ); break;
			}

			EVP_CIPHER_CTX * e = (EVP_CIPHER_CTX *) malloc ( sizeof(EVP_CIPHER_CTX) );
			EVP_CIPHER_CTX * d = (EVP_CIPHER_CTX *) malloc ( sizeof(EVP_CIPHER_CTX) );
			if ( !e || !d ) {
				CErr ( "AESDeriveKeyContexts: Failed to allocate memory for enc/dec contexts." );
                if ( e ) free ( e );
                if ( d ) free ( d );
                break;
			}

			ctx->encCtx = (char *)e;
			ctx->decCtx = (char *)d;

			unsigned char hash [ SHA256_DIGEST_LENGTH ];
			Zero ( hash );

			dSHA256_Init ( &sha );
			dSHA256_Update ( &sha, key, AES_SHA256_KEY_LENGTH );

			dSHA256_Final ( hash, &sha );
			memcpy ( blob, hash, AES_SHA256_KEY_LENGTH );
			
			dEVP_CIPHER_CTX_init ( e );
			ret = (dEVP_EncryptInit_ex ( e, dEVP_aes_256_cbc(), NULL, (unsigned char *) blob, NULL ) == 1);
            if ( !ret ) {
                CWarn ( "AESDeriveKeyContexts: Failed to initialize encryption." );
            }

			dEVP_CIPHER_CTX_init ( d );
            ret = (dEVP_DecryptInit_ex ( d, dEVP_aes_256_cbc(), NULL, (unsigned char *) blob, NULL ) == 1);
            if ( !ret ) {
                CWarn ( "AESDeriveKeyContexts: Failed to decrypt." );
            }
    
			CVerbVerbArg ( "AESDeriveKeyContexts: AES key [%s]", ConvertToHexSpaceString ( blob, AES_SHA256_KEY_LENGTH ) );

			ctx->size = AES_SHA256_KEY_LENGTH;
			ret = true;
		}
		while ( 0 );
#else
		HCRYPTPROV		hCSP		= 0;
		HCRYPTKEY		hKey		= 0;
		HCRYPTHASH		hHash       = 0;
		DWORD			blobLen		= 0;

		do
		{
			if ( !CryptAcquireContext ( &hCSP, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT ) ) 
			{
				CErr ( "AESDeriveKeyContexts: CryptAcquireContext failed." ); break;
			}

			if ( !CryptCreateHash ( hCSP, CALG_SHA_256, 0, 0, &hHash ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptCreateHash failed." ); break;
			}

			if ( !CryptHashData ( hHash, (const BYTE *)key, AES_SHA256_KEY_LENGTH, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptHashData failed." ); break;
			}

			//if ( !CryptDeriveKey ( hCSP, CALG_AES_256, hHash, 0x00800000 /*128 bit*/, &hKey ) )
			if ( !CryptDeriveKey ( hCSP, CALG_AES_256, hHash, 0x01000000 /*256 bit*/, &hKey ) )//CRYPT_EXPORTABLE | 
			{
				CErr ( "AESDeriveKeyContexts: CryptDeriveKey failed." ); break;
			}

			/*DWORD cryptMode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKey, KP_MODE, (BYTE*) &cryptMode, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
			}*/

			DWORD mode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKey, KP_MODE, (BYTE*) &mode, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
			}

			mode = PKCS5_PADDING;
			if ( !CryptSetKeyParam ( hKey, KP_PADDING, (BYTE*) &mode, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
			}

			DWORD blobLenLen = sizeof(DWORD);
			if ( !CryptGetHashParam ( hHash, HP_HASHSIZE, (BYTE *) &blobLen, &blobLenLen, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptGetHashParam HP_HASHSIZE failed." ); break;
			}

			blob = (char *) malloc ( blobLen );
			if ( !blob ) {
				CErr ( "AESDeriveKeyContexts: Failed to allocate memory for hashed blob." ); break;
			}

			if ( !CryptGetHashParam ( hHash, HP_HASHVAL, (BYTE *) blob, &blobLen, 0 ) )
			{
				CErr ( "AESDeriveKeyContexts: CryptGetHashParam HP_HASHVAL failed." ); break;
			}

			CVerbVerbArg ( "AESDeriveKeyContexts: AES key [%s]", ConvertToHexSpaceString ( blob, AES_SHA256_KEY_LENGTH ) );

#ifdef USE_CACHED_HKEY
			ctx->keyCtx = (char *) hCSP;
			hCSP = 0;
			ctx->encCtx = (char *) hKey;
			ctx->decCtx = (char *) hKey;
			ctx->size = blobLen;
			hKey = 0;	
#else
			ctx->encCtx = blob;
			ctx->decCtx = blob;
			ctx->size = blobLen;
			blob = 0;		
#endif
			ret = true;
		} 
		while ( 0 );

		if ( hHash )	CryptDestroyHash ( hHash );
		if ( hKey )		CryptDestroyKey ( hKey );
		if ( hCSP )		CryptReleaseContext ( hCSP, 0 );
#endif
		if ( blob )		free ( blob );
		
		return ret;
    }

    
#ifdef ENABLE_DUMMY_CRYPT
    bool dAESEncrypt ( AESContext * ctx, char * buffer, unsigned int *bufferLen, char ** cipher )
    {
        return false;
    }
#endif
    
    /*
     * Encrypt *len bytes of data
     * All data going in & out is considered binary (unsigned char[])
     */
	bool cryptAESEncrypt ( AESContext * ctx, char * buffer, unsigned int *bufferLen, char ** cipher )
	{
		CVerbVerb ( "AESEncrypt" );

		if ( !ctx || !ctx->encCtx || !buffer || !bufferLen || !cipher ) {
			CErr ( "AESEncrypt: Called with at least one NULL argument." ); return false;
		}

		bool			ret			= false;
		char		*	ciphers		= 0;
		char		*	cipherStart = 0;
		unsigned int	ciphersSize = 0;
        unsigned int    deviceID    = ctx->deviceID;
		
		//CVerbVerbArgID ( "AESEncrypt: [%s]", buffer + 4 );
		//CVerbVerbArgID ( "AESEncrypt: [%s]", ConvertToHexSpaceString ( buffer, *bufferLen ) );

#ifdef USE_OPENSSL_AES
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		if ( pthread_mutex_lock ( &ctx->encLock ) ) {
			CErrID ( "AESEncrypt: Failed to acquire mutex." );
			return false;
		}
#endif
		do
		{
			EVP_CIPHER_CTX * e = (EVP_CIPHER_CTX *) ctx->encCtx;

			/* max ciphertext len for a n bytes of plaintext is n + AES_BLOCK_SIZE -1 bytes */
			ciphersSize = *bufferLen + AES_SHA256_KEY_LENGTH;
            
            CVerbVerbArg ( "AESEncrypt: ciphersSize [%i]", ciphersSize );
            
			int finalSize = 0;
			ciphers = (char *) malloc ( ciphersSize + 21 );
			if ( !ciphers ) {
				CErrArgID ( "AESEncrypt: Memory allocation [%i bytes] failed.", ciphersSize ); break;
			}
        
            BUILD_IV_128 ( ciphers + 4 );


			cipherStart = ciphers + 20;
            
            CVerbVerb ( "AESEncrypt: dEVP_EncryptInit_ex" );
            
            if ( dEVP_EncryptInit_ex ( e, NULL, NULL, NULL, (unsigned char *) (ciphers + 4) ) != 1 ) {
				CErrID ( "AESEncrypt: EVP_EncryptInit_ex IV failed." ); break;
            }

			/*if ( !dEVP_EncryptInit_ex ( e, NULL, NULL, NULL, NULL ) ) {
				CErrID ( "AESEncrypt: EVP_EncryptInit_ex failed." ); break;
			}
            */
            
            CVerbVerbArg ( "AESEncrypt: bufferLen [%i]", *bufferLen );
            
			if ( dEVP_EncryptUpdate ( e, (unsigned char *)cipherStart, (int *)&ciphersSize, (unsigned char *)buffer, *bufferLen ) != 1 ) {
				CErrID ( "AESEncrypt: EVP_EncryptUpdate failed." ); break;
			}
        
            /* update ciphertext with the final remaining bytes */
            CVerbVerbArg ( "AESEncrypt: ciphersSize [%i]", ciphersSize );
            
			if ( dEVP_EncryptFinal_ex ( e, (unsigned char *) (cipherStart + ciphersSize), &finalSize ) != 1 ) {
				CErrID ( "AESEncrypt: EVP_EncryptFinal_ex failed." ); break;
            }
            
            CVerbVerbArg ( "AESEncrypt: finalSize [%i]", finalSize );
       
			ciphersSize += finalSize;
			ret = true;
		}
		while ( 0 );
        
#   ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		if ( pthread_mutex_unlock ( &ctx->encLock ) ) {
			CErrID ( "AESEncrypt: Failed to release mutex." );
			ret = false;
		}
#   endif
        
#else
#   ifdef USE_CACHED_HKEY
		HCRYPTKEY		hKey		= (HCRYPTKEY) ctx->encCtx;
#   else
		HCRYPTPROV		hCSP		= 0;
		HCRYPTKEY		hKey		= 0;
		AES_256_BLOB blob = {
			{ PLAINTEXTKEYBLOB, CUR_BLOB_VERSION, 0, CALG_AES_256 },
			32,
			{ },
		};
		memcpy ( &blob.raw, ctx->encCtx, ctx->size );
#   endif
        
#   ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		if ( pthread_mutex_lock ( &ctx->decLock ) ) {
			CErrID ( "AESEncrypt: Failed to acquire mutex." );
			return false;
		}
#   endif
        
		do
		{
#   ifndef USE_CACHED_HKEY
			if ( !CryptAcquireContext ( &hCSP, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT ) )
			{
				CErrID ( "AESEncrypt: CryptAcquireContext failed." ); break;
			}

			if ( !CryptImportKey ( hCSP, (BYTE *)&blob, sizeof(blob), NULL, 0, &hKey ) )
			{
				CErrID ( "AESEncrypt: CryptImportKey failed." ); break;
			}
#   endif
			/*DWORD param = 0;
			DWORD paramSize = sizeof(param);
			if ( !CryptGetKeyParam ( hKey, KP_BLOCKLEN, (BYTE*) &param, &paramSize, 0 ) )
			{
				CErrID ( "AESEncrypt: CryptGetKeyParam KP_BLOCKLEN failed." ); break;
			}
			CVerbVerbArgID ( "AESEncrypt: Blocklen [%u]", param );*/

			/*DWORD mode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKey, KP_MODE, (BYTE*) &mode, 0 ) )
			{
				CErrID ( "AESEncrypt: CryptSetKeyParam failed." ); break;
			}

			mode = PKCS5_PADDING;
			if ( !CryptSetKeyParam ( hKey, KP_PADDING, (BYTE*) &mode, 0 ) )
			{
				CErrID ( "AESEncrypt: CryptSetKeyParam failed." ); break;
			}*/

			/*DWORD reqSize = (DWORD) *bufferLen;
			if ( !CryptEncrypt ( hKey, NULL, TRUE, 0, 0, &reqSize, reqSize ) || !reqSize )
			{
				CErrID ( "AESEncrypt: CryptEncrypt retrieve cipher text size failed ." ); break;
			}*/
			DWORD reqSize = (DWORD) *bufferLen;
			reqSize += AES_256_BLOCK_SIZE;

			ciphers = (char *) malloc ( reqSize + 21 );
			if ( !ciphers ) {
				CErrArgID ( "AESEncrypt: Memory allocation [%i bytes] failed.", reqSize ); break;
			}

            BUILD_IV_128 ( ciphers + 4 );

			if ( !CryptSetKeyParam ( hKey, KP_IV, (BYTE*) (ciphers + 4), 0 ) ) {
				CErrID ( "AESEncrypt: CryptSetKeyParam KP_IV failed." ); break;
			}
			
			DWORD ciphersLen = (DWORD) *bufferLen;
			cipherStart = ciphers + 20;
			memcpy ( cipherStart, buffer, ciphersLen );

			if ( !CryptEncrypt ( hKey, NULL, TRUE, 0, (BYTE *)cipherStart, &ciphersLen, reqSize ) )
			{
				CErrID ( "AESEncrypt: CryptEncrypt failed." ); break;
			}

			ciphersSize = ciphersLen;
			ret = true;
		}
		while ( 0 );

#   ifndef USE_CACHED_HKEY
		if ( hKey )		CryptDestroyKey ( hKey );
		if ( hCSP )		CryptReleaseContext ( hCSP, 0 );
#   endif
        
#   ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		if ( pthread_mutex_unlock ( &ctx->decLock ) ) {
			CErrID ( "AESEncrypt: Failed to release mutex." );
			ret = false;
		}
#   endif
#endif
		if ( ret ) {
			cipherStart [ ciphersSize ] = 0;
			ciphersSize += 20;

			/// Update enc header
			*((unsigned int *) ciphers) = (0x40000000 | ciphersSize);
		
			//CVerbVerbArgID ( "AESEncrypt: [%s]", ConvertToHexSpaceString ( ciphers, ciphersSize ) );

			*cipher = ciphers;
			//CVerbVerbArgID ( "AESEncrypt: IV [%s]", ConvertToHexSpaceString ( ciphers + 4, 16 ) );

			ciphers = 0;
			*bufferLen = ciphersSize;
		}

		if ( ciphers ) free ( ciphers );
		return ret;
    }
    
    
#ifdef ENABLE_DUMMY_CRYPT
    bool dAESDecrypt ( AESContext * ctx, char * buffer, unsigned int *bufferLen, char ** decrypted )
    {
        return false;
    }
#endif

	bool cryptAESDecrypt ( AESContext * ctx, char * buffer, unsigned int *bufferLen, char ** decrypted )
	{
		CVerbVerb ( "AESDecrypt" );

		if ( !ctx || !ctx->decCtx || !buffer || !bufferLen || *bufferLen < 36 || !decrypted ) {
			CErr ( "AESDecrypt: Called with at least one NULL argument or encrypted packet is < 36." ); return false;
		}
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		if ( pthread_mutex_lock ( &ctx->decLock ) ) {
			CErr ( "AESDecrypt: Failed to acquire mutex." );
			return false;
		}
#endif
		bool			ret				= false;
		char		*	decrypt			= 0;
        char        *   IV              = buffer + 4;
		unsigned int	decryptedBufSize = *bufferLen + AES_SHA256_KEY_LENGTH + 5;
        unsigned int    deviceID        = ctx->deviceID;

		//CVerbVerbArgID ( "AESDecrypt: IV [%s]", ConvertToHexSpaceString ( IV, 16 ) );
		
		//CVerbVerbArgID ( "AESDecrypt: [%s]", ConvertToHexSpaceString ( buffer, *bufferLen ) );

#ifdef USE_OPENSSL_AES
		unsigned int	decryptedSize	= 0;

		do
		{
			EVP_CIPHER_CTX * e = (EVP_CIPHER_CTX *) ctx->decCtx;			

			decryptedSize = *bufferLen;
			int finalSize = 0;
			decrypt = (char *) malloc ( decryptedBufSize + 2 );
			if ( !decrypt ) {
				CErrArg ( "AESDecrypt: Memory allocation [%i bytes] failed.", decryptedBufSize ); break;
			}
            
            if ( dEVP_DecryptInit_ex ( e, NULL, NULL, NULL, (unsigned char *) IV ) != 1 ) {
                CErrID ( "AESDecrypt: EVP_DecryptInit_ex for IV failed." ); break;
            }
            
            unsigned int len = *bufferLen - 20;
            if ( dEVP_DecryptUpdate ( e, (unsigned char *)decrypt, (int *)&decryptedSize, (unsigned char *)(buffer + 20), len ) != 1 ) {
                CErrID ( "AESDecrypt: EVP_DecryptUpdate failed." ); break;
            }
            
			if ( dEVP_DecryptFinal_ex ( e, (unsigned char *) (decrypt + decryptedSize), &finalSize ) != 1 ) {
				CErrID ( "AESDecrypt: EVP_DecryptFinal_ex failed." ); break;
			}

			decryptedSize += finalSize;
			ret = true;
		}
		while ( 0 );
#else
		
		DWORD			decryptedSize	= 0;
#ifdef USE_CACHED_HKEY
		HCRYPTKEY		hKey			= (HCRYPTKEY)ctx->decCtx;
#   else
		HCRYPTKEY		hKey			= 0;
		HCRYPTPROV		hCSP			= 0;
		
		AES_256_BLOB blob = {
			{ PLAINTEXTKEYBLOB, CUR_BLOB_VERSION, 0, CALG_AES_256 },
			32,
			{ },
		};
		memcpy ( &blob.raw, ctx->decCtx, ctx->size );
#endif
		do
		{
#ifndef USE_CACHED_HKEY
			if ( !CryptAcquireContext ( &hCSP, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT ) )
			{
				CErrID ( "AESDecrypt: CryptAcquireContext failed." ); break;
			}

			if ( !CryptImportKey ( hCSP, (BYTE *)&blob, sizeof(blob), NULL, 0, &hKey ) )
			{
				CErrID ( "AESDecrypt: CryptImportKey failed." ); break;
			}

			/*DWORD cryptMode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKey, KP_MODE, (BYTE*) &cryptMode, 0 ) )
			{
				CErrID ( "AESDeriveKeyContexts: CryptSetKeyParam failed." ); break;
				}*/
#endif
			/*DWORD param = 0;
			DWORD paramSize = sizeof(param);
			if ( !CryptGetKeyParam ( hKey, KP_BLOCKLEN, (BYTE*) &param, &paramSize, 0 ) )
			{
				CErrID ( "AESDecrypt: CryptGetKeyParam KP_BLOCKLEN failed." ); break;
			}
			CVerbVerbArgID ( "AESDecrypt: Blocklen [%u]", param );*/

			/*DWORD mode = CRYPT_MODE_CBC;
			if ( !CryptSetKeyParam ( hKey, KP_MODE, (BYTE*) &mode, 0 ) )
			{
				CErrID ( "AESDecrypt: CryptSetKeyParam KP_MODE failed." ); break;
			}

			mode = PKCS5_PADDING;
			if ( !CryptSetKeyParam ( hKey, KP_PADDING, (BYTE*) &mode, 0 ) )
			{
				CErrID ( "AESDecrypt: CryptSetKeyParam KP_PADDING failed." ); break;
			}*/

			decrypt	= (char *) malloc ( decryptedBufSize + 2 );
			if ( !decrypt ) {
				CErrArgID ( "AESDecrypt: Memory allocation [%i bytes] failed.", decryptedBufSize ); break;
			}

			if ( !CryptSetKeyParam ( hKey, KP_IV, (BYTE *)IV, 0 ) ) {
				CErrID ( "AESDecrypt: CryptSetKeyParam KP_IV failed." ); break;
			}

			decryptedSize = *bufferLen - 20;
			memcpy ( decrypt, buffer + 20, decryptedSize );

			if ( !CryptDecrypt ( hKey, NULL, TRUE, 0, (BYTE *)decrypt, &decryptedSize ) ) {
				CErrID ( "AESDecrypt: CryptDecrypt failed." ); break;
			}

			ret = true;
		} 
		while ( 0 );

		if ( !ret ) {
			DWORD err = GetLastError ( );
			CErrArgID ( "AESDecrypt: Last error [0x%08x] [%u]", err, err );
		}

#   ifndef USE_CACHED_HKEY
		if ( hKey )		CryptDestroyKey ( hKey );
		if ( hCSP )		CryptReleaseContext ( hCSP, 0 );
#   endif
#endif
		if ( ret ) {
			if ( decryptedSize >= decryptedBufSize ) {
				CErrArgID ( "AESDecrypt: Decrypted message size [%u] > buffer size [%u]", decryptedSize, decryptedBufSize );
				ret = false;
			}
			else {
				//CVerbVerbArgID ( "AESDecrypt: [%s]", ConvertToHexSpaceString ( decrypt, decryptedSize ) );

				*decrypted = decrypt;
				decrypt [ decryptedSize ] = 0;
				decrypt = 0;
				*bufferLen = decryptedSize;
			}
		}
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		if ( pthread_mutex_unlock ( &ctx->decLock ) ) {
			CErrID ( "AESDecrypt: Failed to release mutex." );
			ret = false;
		}
#endif

		if ( decrypt ) free ( decrypt );
		return ret;
    }
    
#ifdef ENABLE_DUMMY_CRYPT
    bool dSHAHashCreate ( const char * msg, char ** hash, unsigned int * xchLen )
    {
        return false;
    }
#endif

	/**
	*	SHAHashCreate: Creates a SHA512 hash for a given message, i.e. used to hash user passwords.
	*
	*/
	bool cryptSHAHashCreate ( const char * msg, char ** hash, unsigned int * xchLen )
	{
		CVerb ( "SHAHashCreate" );

		if ( !msg || !hash || !xchLen ) {
			CErr ( "SHAHashCreate: Invalid params." );
            return false;
		}
        
		bool		ret		= false;
		char	*	blob	= 0;
		

#ifdef _WIN32
		HCRYPTPROV			hCSP		= 0;
		HCRYPTHASH			hHash       = 0;
		DWORD				blobLen		= 0;
		HCRYPTKEY			hSessKey	= 0;

		do
		{
			if ( !CryptAcquireContext ( &hCSP, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT ) )
			{
				CErr ( "SHAHashCreate: CryptAcquireContext failed." ); break;
			}

			//ALG_ID  algorithmID = CALG_SHA1; CALG_SHA_256
			if ( !CryptCreateHash ( hCSP, CALG_SHA_512, 0, 0, &hHash ) )
			{
				CErr ( "SHAHashCreate: CryptCreateHash failed." ); break;
			}

			if ( !CryptHashData ( hHash, (const BYTE *) msg, *xchLen, 0 ) )
			{
				CErr ( "SHAHashCreate: CryptHashData failed." ); break;
			}

			if ( !CryptGetHashParam ( hHash, HP_HASHVAL, 0, &blobLen, 0 ) || !blobLen )
			{
				CErr ( "SHAHashCreate: Get hash value length failed." ); break;
			}

			blob = (char *) malloc ( blobLen + 1 );
			if ( !blob ) {
				CErr ( "SHAHashCreate: Failed to allocate memory for hashed blob." ); break;
			}

			if ( !CryptGetHashParam ( hHash, HP_HASHVAL, (BYTE*) blob, &blobLen, 0 ) )
			{
				CErr ( "SHAHashCreate: Retrieve hash value failed" ); break;
			}
			blob [ blobLen ] = 0;

			*hash = blob;
			blob = 0;
			*xchLen = blobLen;
			ret = true;

			if ( !ret ) {
				CErrArg ( "SHAHashCreate: Last error [0x%08x]", GetLastError ( ) );
			}
		}
		while ( 0 );

		if ( hSessKey ) CryptDestroyKey ( hSessKey );
		if ( blob )		free ( blob );
		if ( hHash )	CryptDestroyHash ( hHash );
		if ( hCSP )		CryptReleaseContext ( hCSP, 0 );

#else
		
#ifdef USE_OPENSSL
        SHA512_CTX ctx;
		do
		{
			if ( !dSHA512_Init ( &ctx ) ) {
				CErr ( "SHAHashCreate: SHA512_Init failed." ); break;
			}

			if ( !dSHA512_Update ( &ctx, msg, *xchLen ) ) {
				CErr ( "SHAHashCreate: SHA512_Update failed." ); break;
			}

			blob = (char *) malloc ( SHA512_DIGEST_LENGTH + 1 );
			if ( !blob ) {
				CErr ( "SHAHashCreate: Memory allocation failed." ); break;
			}

			if ( !dSHA512_Final ( (unsigned char *)blob, &ctx ) ) {
				CErr ( "SHAHashCreate: SHA512_Final failed." ); break;
			}            

			blob [ SHA512_DIGEST_LENGTH ] = 0;

			*hash = blob;
			blob = 0;
			*xchLen = SHA512_DIGEST_LENGTH;
			ret = true;
		}
		while ( 0 );

		if ( blob )
			free ( blob );

#endif /// -> USE_OPENSSL

#endif /// -> _WIN32

		return ret;
	}
#endif

	
	/**
	*	SHAHashPassword: Creates a SHA512 hashed and salted digest for a given password.
	*
	*/
	bool SHAHashPassword ( const char * pass, char ** hash, unsigned int * xchLen )
	{
		CVerb ( "SHAHashPassword" );

		if ( !pass || !xchLen ) {
			CErr ( "SHAHashPassword: Invalid params." );
			return false;
		}
		
		unsigned int len = (unsigned int) strlen ( pass );

		char saltedSoup [MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 1];
		memcpy ( saltedSoup, pass, len );

		unsigned int saltLen = (unsigned int) strlen ( hashSalt );
		if ( saltLen + len > MAX_NAMEPROPERTY + MAX_NAMEPROPERTY )
			saltLen = (MAX_NAMEPROPERTY + MAX_NAMEPROPERTY) - len;

		memcpy ( saltedSoup + len, hashSalt, saltLen );

		len += saltLen;
		saltedSoup [len] = 0;

		*xchLen = len;
		return SHAHashCreate ( saltedSoup, hash, xchLen );
	}


	/**
	*	BuildEnvironsResponse: Creates a response hash and prepent the username
	*		Format: Length(4) userName(x) Length(4) hash
	*
	*/
	bool BuildEnvironsResponse ( unsigned int response, const char * userName, const char * userPass, char ** hash, unsigned int * hashLen )
	{
		CVerb ( "BuildEnvironsResponse" );

		char buffer [ENVIRONS_USER_PASSWORD_LENGTH + (MAX_NAMEPROPERTY * 8) + 12];

		unsigned int * pUI = (unsigned int *) buffer;

		*pUI = response;
		unsigned int responseLen = 4;

		if ( userName && *userName && userPass && *userPass ) {
			CVerb ( "BuildEnvironsResponse: using user/pass to secure the response" );
			unsigned int length = (unsigned int) strlen ( userName );
			memcpy ( buffer + responseLen, userName, length );
			responseLen += length;

			length = (unsigned int) strlen ( userPass );
			if ( length > ENVIRONS_USER_PASSWORD_LENGTH )
				return false;

			memcpy ( buffer + responseLen, userPass, length );
			responseLen += length;
		}
		else {
			/// fill in random data 
			CVerb ( "BuildEnvironsResponse: using random data to secure the response" );
			pUI++;
			unsigned int ints = ENVIRONS_USER_PASSWORD_LENGTH / 4;
			for ( unsigned int i=0; i < ints; i++ ) {
				*pUI = rand (); pUI++;
			}
			responseLen += ints;
		}
		*hashLen = responseLen;

		return SHAHashCreate ( (const char *) buffer, hash, hashLen );

#ifdef DEBUGCIPHERS_
		char v = 'a';
		char * ciphers = (char *) calloc ( 1, 65 );
		if ( !ciphers )
			return false;

		for ( int i=0; i<64; i++) {
			ciphers[i] = v;
			v++;
			if ( v == 'z' )
				v = 'a';
		}
		*hash = ciphers;
		*hashLen  = 64;
		return true;
#else
#endif
	}

    
#ifdef USE_OPENSSL
	void cryptDisposePrivateKey ( void ** key )
	{
        if ( key && *key ) {
            dRSA_free ( (RSA *)*key );
            *key = 0;
        }
    }
#endif
    
    
    void dDisposePrivateKey ( void ** key )
    {
        if ( key && *key ) {
            unsigned int keySize = *((unsigned int *)(*key));
            memset ( *key, 0, keySize );
            free ( *key );
            *key = 0;
        }
    }
    
    
#ifdef USE_OPENSSL
	void cryptDisposePublicKey ( void * key )
	{
        if ( key ) dRSA_free ( (RSA *)key );
    }
#endif
    
    void dDisposePublicKey ( void * key )
    {
        if ( key )  free ( key );
    }


#ifdef MEDIATORDAEMON

	bool LoadPublicCertificate ( const char * pathFile, char ** cert )
	{
		if ( !pathFile ) {
			CErr ( "LoadPublicCertificate: Called with NULL argument for path and file name" );
			return false;
		}

		if ( *cert ) free ( *cert );
		*cert = 0;

		bool ret = false;
        
#ifdef USE_OPENSSL
        X509            *   cert509		= 0;
        unsigned char   *   certData	= 0;
        int                 certDataSize = 0;
        
        FILE * fp = fopen ( pathFile, "rb" );
        
        do
        {
            CLogArg ( "LoadPublicCertificate: [%s]", pathFile );
            
            if ( !fp ) {
                CErr ( "LoadPublicCertificate: 404." ); break;
            }
            
            if ( !dPEM_read_X509 ( fp, &cert509, NULL, NULL) ) {
                CErr ( "LoadPublicCertificate: Read X509 cert file failed." );
                dERR_print_errors_fp ( stderr );
                break;
            }

            certDataSize = di2d_X509 ( cert509, 0 );
            if ( certDataSize <= 0 ) {
                CErrArg ( "LoadPublicCertificate: i2d_X509 returned size [%i].", certDataSize ); break;
            }
            
            certData = (unsigned char *) malloc ( certDataSize + 4 );
            if ( !certData ) {
                CErrArg ( "LoadPublicCertificate: Memory allocation failed. size [%i].", certDataSize ); break;
            }
            
            unsigned char * certDataStore = certData + 4;
            if ( !di2d_X509 ( cert509, &certDataStore ) ) {
                CErr ( "LoadPublicCertificate: i2d_X509 failed." ); break;
            }
            *((unsigned int *) certData) = ('d' << 16 | certDataSize );
            *cert = (char *) certData;
            certData = 0;
            ret = true;
        }
        while ( 0 );
        
        if ( certData )
            free ( certData );
        if ( cert509 )
            dX509_free ( cert509 );
        if ( fp )
            fclose ( fp );
        if ( !ret ) {
            if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
            else
                if ( dERR_remove_state ) dERR_remove_state ( 0 );
        }
#else       
		int certSize = 0;
		char * certBin = LoadBinary ( pathFile, &certSize );
		if ( !certBin ) {
			CErrArg ( "LoadPublicCertificate: Failed to load [%s]", pathFile );
			return false;
		}
        
		if ( !certSize ) {
			CErrArg ( "LoadPublicCertificate: Invalid (Zero) sized file loaded [%s]", pathFile );
			free ( certBin );
			return false;
		}

		BYTE	*	certBuffer = 0;

		do
		{
			DWORD certBufferSize = 0;

			if ( !CryptStringToBinaryA ( certBin, certSize, CRYPT_STRING_ANY, 0, &certBufferSize, NULL, NULL ) || !certBufferSize )
			{
				CErr ( "LoadPublicCertificated: CryptStringToBinaryA (retrieve size) failed." ); break;
			}

			certBuffer = (BYTE *)malloc ( certBufferSize + 4 );
			if ( !certBuffer ) {
				CErrArg ( "LoadPublicCertificated: Memory alloc failed [%u].", certBufferSize + 4 ); break;
			}

			if ( !CryptStringToBinaryA ( certBin, certSize, CRYPT_STRING_ANY, certBuffer + 4, &certBufferSize, NULL, NULL ) || !certBufferSize )
			{
				CErr ( "LoadPublicCertificated: CryptStringToBinaryA (convert) failed." ); break;
			}
			
			*((unsigned int *) certBuffer) = (('d' << 16) | certBufferSize );
			*cert = (char *) certBuffer;
			certBuffer = 0; 
			ret = true;
		}
		while ( 0 );
		
		if ( !ret ) {
			CErrArg ( "LoadPublicCertificated: Last error [0x%08x]", GetLastError ( ) );
		}

		if ( certBuffer )     free ( certBuffer );
		if ( certBin )   free ( certBin );
#endif
		return ret;
	}

	
	bool LoadPrivateKey ( const char * pathFile, char ** key, unsigned int * keySize )
	{
		if ( !pathFile ) {
			CErr ( "LoadPrivateKey: Called with NULL argument for path and file name" ); return false;
		}

		bool ret = false;

		CLogArg ( "LoadPrivateKey: [%s]", pathFile );
		
#ifdef USE_OPENSSL
        RSA * rsaKey = 0;

		FILE * fp = fopen ( pathFile, "rb" );
		if ( !fp ) {
			CErr ( "LoadPrivateKey: 404." );
			goto Finish;
		}

		if ( !dPEM_read_RSAPrivateKey ( fp, &rsaKey, NULL, NULL) )
		{
			CErr ( "LoadPrivateKey: Read RSA file failed." );
            dERR_print_errors_fp ( stderr );
			goto Finish;
        }

        *key = (char *) rsaKey;
        rsaKey = 0;
		ret = true;

	Finish:
		if ( rsaKey )
			dRSA_free ( rsaKey );
		if ( fp )
			fclose ( fp );
        if ( !ret ) {
            if ( dERR_remove_thread_state ) dERR_remove_thread_state ( 0 );
            else
                if ( dERR_remove_state ) dERR_remove_state ( 0 );
        }
#else
		int size = 0;
		char * keyBin = LoadBinary ( pathFile, &size );
		if ( !keyBin ) {
			CErrArg ( "LoadPrivateKey: Failed to load [%s]", pathFile );
			return false;
		}

		if ( !size ) {
			CErrArg ( "LoadPrivateKey: Invalid (Zero) sized file [%s]", pathFile );
			free ( keyBin );
			return false;
		}

		BYTE		*	keyBlob		= 0;
		DWORD			keyBlobSize = 0;
		BYTE		*	keyData		= 0;
		DWORD			keyDataSize = 0;
		char		*	keyRet		= 0;

		// PEM - Base64 to DER
		if ( !CryptStringToBinaryA ( keyBin, size, CRYPT_STRING_BASE64HEADER, 0, &keyDataSize, NULL, NULL) || !keyDataSize )
		{
			CErr ( "LoadPrivateKey: CryptStringToBinaryA (retrieve size)" );
			goto Finish;
		}

		keyData = (BYTE *) malloc ( keyDataSize );
		if ( !keyData || !CryptStringToBinaryA ( keyBin, size, CRYPT_STRING_ANY, keyData, &keyDataSize, NULL, NULL) || !keyDataSize )
		{
			CErr ( "LoadPrivateKey: CryptStringToBinaryA (convert)" );
			goto Finish;
		}

		if ( !CryptDecodeObjectEx ( X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY, keyData, keyDataSize, CRYPT_DECODE_ALLOC_FLAG, NULL, &keyBlob, &keyBlobSize ) )
		{
			CErr ( "LoadPrivateKey: CryptDecodeObjectEx (retrieve size)" );
			goto Finish;
		}

		keyRet = (char *)malloc ( keyBlobSize );
		if ( !keyRet ) {
			CErr ( "LoadPrivateKey: Failed to allocate memory for key." );
			goto Finish;
		}

		memcpy ( keyRet, keyBlob, keyBlobSize );
		*key = keyRet;
		keyRet = 0;
		*keySize = keyBlobSize;


		ret = true;

	Finish:

		if ( !ret ) {
			CErrArg ( "LoadPrivateKey: Last error [0x%08x]", GetLastError ( ) );
		}

		if ( keyData )	free ( keyData );
		if ( keyBlob )	LocalFree ( keyBlob );
		if ( keyRet )	free ( keyRet );
		if ( keyBin )	free ( keyBin );
#endif

		return ret;
	}
#endif
    
    
    void ReleaseEnvironsCrypt ()
    {
        if ( allocated )
        {
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
            MutexDispose ( &privKeyMutex );
            
            CryptLocksDispose ();
#endif
            allocated = false;
        }
        
        ReleaseLibOpenSSL();
    }
    
    
    bool InitEnvironsCrypt ()
    {
#ifdef USE_OPENSSL
        
        if ( InitLibOpenSSL ( 0 ) ) {
            CInfo ( "InitEnvironsCrypt: Successfuly loaded libcrypto!" );
            
            EncryptMessage = cryptEncryptMessage;
            DecryptMessage = cryptDecryptMessage;
            ReleaseCert = cryptReleaseCert;
            SHAHashCreate = cryptSHAHashCreate;
            AESEncrypt = cryptAESEncrypt;
            AESDecrypt = cryptAESDecrypt;
            GenerateCertificate = cryptGenerateCertificate;
            AESDeriveKeyContext = cryptAESDeriveKeyContext;
            AESUpdateKeyContext = cryptAESUpdateKeyContext;
            AESDisposeKeyContext = cryptAESDisposeKeyContext;
            PreparePrivateKey   = cryptPreparePrivateKey;
            DisposePublicKey   = cryptDisposePublicKey;
            DisposePrivateKey = cryptDisposePrivateKey;

            if ( !openssl_alg_added )
            {
				if ( dOpenSSL_add_all_algorithms )
					dOpenSSL_add_all_algorithms ();
				else
#   ifdef OPENSSL_LOAD_CONF
					if ( dOPENSSL_add_all_algorithms_conf )
						dOPENSSL_add_all_algorithms_conf();
#   else
					if ( dOPENSSL_add_all_algorithms_noconf )
						dOPENSSL_add_all_algorithms_noconf();
#   endif
                dERR_load_crypto_strings ();
            
                openssl_alg_added = true;
            }
        }
        else {
#   ifndef ANDROID
            CErr ( "InitEnvironsCrypt: CRITICAL ERROR. Failed to load libcrypto! Encryption not available!" );
#   endif
            EncryptMessage = dEncryptMessage;
            DecryptMessage = dDecryptMessage;
            ReleaseCert = dReleaseCert;
            SHAHashCreate = dSHAHashCreate;
            AESEncrypt = dAESEncrypt;
            AESDecrypt = dAESDecrypt;
            GenerateCertificate = dGenerateCertificate;
            AESDeriveKeyContext = dAESDeriveKeyContext;
            AESUpdateKeyContext = dAESUpdateKeyContext;
            AESDisposeKeyContext = dAESDisposeKeyContext;
            PreparePrivateKey   = dPreparePrivateKey;
            DisposePublicKey   = dDisposePublicKey;
            DisposePrivateKey = dDisposePrivateKey;
        }
#else
#   ifdef _WIN32
		EncryptMessage = cryptEncryptMessage;
		DecryptMessage = cryptDecryptMessage;
		ReleaseCert = cryptReleaseCert;
		SHAHashCreate = cryptSHAHashCreate;
		AESEncrypt = cryptAESEncrypt;
		AESDecrypt = cryptAESDecrypt;
		GenerateCertificate = cryptGenerateCertificate;
		AESDeriveKeyContext = cryptAESDeriveKeyContext;
		AESUpdateKeyContext = dAESUpdateKeyContext;
		AESDisposeKeyContext = cryptAESDisposeKeyContext;
		PreparePrivateKey   = dPreparePrivateKey;
		DisposePublicKey   = dDisposePublicKey;
		DisposePrivateKey = dDisposePrivateKey;
#   else
        EncryptMessage = dEncryptMessage;
        DecryptMessage = dDecryptMessage;
        ReleaseCert = dReleaseCert;
        SHAHashCreate = dSHAHashCreate;
        AESEncrypt = dAESEncrypt;
        AESDecrypt = dAESDecrypt;
        GenerateCertificate = dGenerateCertificate;
        AESDeriveKeyContext = dAESDeriveKeyContext;
        AESUpdateKeyContext = dAESUpdateKeyContext;
        AESDisposeKeyContext = dAESDisposeKeyContext;
        PreparePrivateKey   = dPreparePrivateKey;
        DisposePublicKey   = dDisposePublicKey;
        DisposePrivateKey = dDisposePrivateKey;
#   endif
#endif
        if ( !allocated )
        {
#ifdef ENABLE_CRYPT_PRIVKEY_LOCKED_ACCESS
            if ( !MutexInit ( &privKeyMutex ) )
                return false;
#endif
            
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
            if ( !CryptLocksCreate () )
                return false;
#endif
            allocated = true;
        }
        return true;
    }
    
}


#ifdef _WIN32
#   pragma warning( pop )
#   pragma warning( pop )
#endif


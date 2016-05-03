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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_CRYPT_UTILS_AND_TOOLS_H
#define INCLUDE_HCM_ENVIRONS_CRYPT_UTILS_AND_TOOLS_H

#include "Environs.Native.h"
#include "Interop/Threads.h"

#if (defined(_WIN32) && !defined(USE_OPENSSL))
#   include <Wincrypt.h>
#endif

#define ENVIRONS_MAX_KEYBUFFER_SIZE		(ENVIRONS_MAX_KEYSIZE + 1024)
#define AES_SHA256_KEY_LENGTH           32
#define AES_256_BLOCK_SIZE				16

#define	ENVIRONS_CRYPT_PAD_FLAG			0xFF000000
#define	ENVIRONS_DEFAULT_CRYPT_PAD		ENVIRONS_CRYPT_PAD_OAEP

#define ENVIRONS_CERT_SIGNED_FLAG       0x80000000

#define ENABLE_CRYPT_PRIVKEY_LOCKED_ACCESS


#if ( defined(ANDROID) || defined(LINUX) || defined(WINDOWS_PHONE) )
#   define USE_OPENSSL
#   define USE_OPENSSL_AES
#else
//#if ( defined(MEDIATORDAEMON) )
    #if ( defined(MEDIATORDAEMON) || defined(ENVIRONS_OSX) )

        #if ( !defined(ENVIRONS_IOS) && !defined(_WIN32) )
        /// Use openssl for macos and mediatordaemon
            #define USE_OPENSSL
            #define USE_OPENSSL_AES
        #endif


    #endif

#endif

#ifndef OPENSSL1
//#	ifndef _WIN32
#		define ENABLE_CRYPT_AES_LOCKED_ACCESS
//#	endif
#endif

#ifdef ENVIRONS_IOS
#   ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
#       undef ENABLE_CRYPT_AES_LOCKED_ACCESS
#   endif
#endif


namespace environs
{
	typedef struct AESContext {
		char		*	keyCtx;
		char		*	encCtx;
        char		*	decCtx;
        
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
		pthread_mutex_t	encLock;
		pthread_mutex_t	decLock;
#endif
        
		unsigned int	size;
        unsigned int    version;
		int				deviceID;
	}
	AESContext;

	extern const char		*	hashSalt;
	extern pthread_mutex_t		privKeyMutex;

    extern bool InitEnvironsCrypt ();
    extern void ReleaseEnvironsCrypt ();

	/// <summary>
	/// The certificate file format is as follows: 
	///	- 4 byte header:
	///		size of the certificate [without the header]
	///		The upper 3rd byte (0x00FF0000) contains the certificate format: d = binary (DER), p = x509 cert file (PEM)
	/// - remaining bytes contains the certificate
	/// </summary>
	extern bool LoadPublicCertificate ( const char * pathFile, char ** cert );
    
    typedef void (CallConv * pDisposePublicKey)(void * key);
    
    extern pDisposePublicKey DisposePublicKey;
	extern void dDisposePublicKey ( void * key );

	/// <summary>
	/// The private key file format is as follows: 
	///	- 4 byte header:
	///		size of the certificate [without the header]
	/// - remaining bytes contains the private key in binary format (DER)
	/// </summary>
	extern bool LoadPrivateKey ( const char * pathFile, char ** keyDER, unsigned int * keyDERSize );
    
    typedef void (CallConv * pDisposePrivateKey)(void ** key);
    
    extern pDisposePrivateKey DisposePrivateKey;
	extern void dDisposePrivateKey ( void ** key );

    extern bool SignCertificate ( char * key, unsigned int keySize, char * pub, char ** cert );
    
    typedef bool (CallConv * pGenerateCertificate)(char ** priv, char ** pub);
    
    extern pGenerateCertificate GenerateCertificate;
    extern bool dGenerateCertificate ( char ** priv, char ** pub );
    
    typedef bool (CallConv * pPreparePrivateKey)(char ** privKey);
    
    extern pPreparePrivateKey PreparePrivateKey;
    extern bool dPreparePrivateKey ( char ** privKey );
    
    extern bool UpdateKeyAndCert ( char * priv, char * cert );

    
    typedef bool (CallConv * pEncryptMessage)(int deviceID, char * cert, char * msg, unsigned int * msgLen);
    
    extern pEncryptMessage EncryptMessage;
	extern bool dEncryptMessage ( int deviceID, char * cert, char * msg, unsigned int * msgLen );
    
    typedef bool (CallConv * pDecryptMessage)(char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize);
    
    extern pDecryptMessage DecryptMessage;
	extern bool dDecryptMessage ( char * key, unsigned int keySize, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize );
    
#ifdef _WIN32    
	typedef bool (CallConv * pDecryptMessageWithKeyHandles )( HCRYPTPROV hCSP, HCRYPTKEY hKey, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize);

	extern pDecryptMessageWithKeyHandles DecryptMessageWithKeyHandles;
	extern bool dDecryptMessageWithKeyHandles ( HCRYPTPROV hCSP, HCRYPTKEY hKey, char * msg, unsigned int msgLen, char ** decrypted, unsigned int * decryptedSize );
#endif

    typedef void (CallConv * pReleaseCert)(int deviceID);
    
    extern pReleaseCert ReleaseCert;
	extern void dReleaseCert ( int deviceID );

    
    typedef bool (CallConv * pSHAHashCreate)(const char * msg, char ** hash, unsigned int * hashLen);
    
    extern pSHAHashCreate SHAHashCreate;
	extern bool dSHAHashCreate ( const char * msg, char ** hash, unsigned int * hashLen );
    
	extern bool SHAHashPassword ( const char * msg, char ** hash, unsigned int * hashLen );
    
	extern bool BuildEnvironsResponse ( unsigned int response, const char * userName, const char * userPass, char ** hash, unsigned int * hashLen );
    
    
    typedef bool (CallConv * pAESDeriveKeyContext)(char * key, unsigned int keyLen, AESContext * ctx);
    
    extern pAESDeriveKeyContext AESDeriveKeyContext;
	extern bool dAESDeriveKeyContext ( char * key, unsigned int keyLen, AESContext * ctx );
    
    typedef void (CallConv * pAESUpdateKeyContext)(AESContext * ctx, int deviceID);
    
    extern pAESUpdateKeyContext AESUpdateKeyContext;
	extern void dAESUpdateKeyContext ( AESContext * ctx, int deviceID );
    
    typedef void (CallConv * pAESDisposeKeyContext)(AESContext * ctx);
    
    extern pAESDisposeKeyContext AESDisposeKeyContext;
	extern void dAESDisposeKeyContext ( AESContext * ctx );
    

#define BUILD_IV_128(pUS)     { unsigned short * dpUS = (unsigned short *)(pUS);\
                              for ( unsigned int i=0; i < 8; i++ ) {\
                                *dpUS = (unsigned short) rand ( ); dpUS++;\
                              } }

    typedef bool (CallConv * pAESEncrypt)(AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** cipher );

    extern pAESEncrypt AESEncrypt;
	extern bool dAESEncrypt ( AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** cipher );


    typedef bool (CallConv * pAESDecrypt)(AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** decrypted);

    extern pAESDecrypt AESDecrypt;
	extern bool dAESDecrypt ( AESContext * ctx, char * buffer, unsigned int * bufferLen, char ** decrypted );
	
	extern const char * ConvertToHexString ( const char * src, unsigned int length );
    extern const char * ConvertToHexSpaceString ( const char * src, unsigned int length );
    extern const char * ConvertToHexSpaceBuffer ( const char * src, unsigned int length, char * buffer, bool limit = true );

	extern const char * ConvertToBytes ( const char * src, unsigned int length );
	extern char * ConvertToNewBytes ( const char * src, unsigned int length );
	extern char * ConvertToByteBuffer ( const char * src, unsigned int length, char * buffer );
}


#endif /// ->INCLUDE_HCM_ENVIRONS_CRYPT_UTILS_AND_TOOLS_H

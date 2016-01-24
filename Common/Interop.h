/**
 * Platform interop declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_INTEROP_H
#define INCLUDE_HCM_ENVIRONS_INTEROP_H

// Determine whether we're compiling for windows 8 and using visual studio > 2010
#if defined(NTDDI_WIN8) && (_MSC_VER >= 1700)
#	define WINDOWS_8
#endif

/**
* Dynamic library interoperability
* - function macros
*/
#if defined(_WIN32)
// dlload defines
#	define dlsym(mod,sym)					GetProcAddress(mod,sym)     
#	define dlopen(mod,RTLD_LAZY)			LoadLibraryA(mod)           
#	define dlclose(mod)						FreeLibrary(mod) 
#else
#	include <dlfcn.h>
#endif

/**
* Misc declarations
*/
#ifdef ANDROID
#    define  JSTRINGNEW(s)					jenv->NewStringUTF ( s )
#    define  JSTRINGNEW_DISPOSE(s)			jstring nst = jenv->NewStringUTF ( s ); free ( s ); return nst
#else
#    define  JSTRINGNEW(s)					s
#    define  JSTRINGNEW_DISPOSE(s)			return s
#endif


#ifdef CLI_CPP
#   define	___sync_val_compare_and_swap(a,c,v)	System::Threading::Interlocked::CompareExchange(a,v,c)

#	define __sync_add_and_fetch(dest,inc)	System::Threading::Interlocked::Increment(dest)

#	define __sync_sub_and_fetch(dest,inc)	System::Threading::Interlocked::Decrement(dest)

#elif (defined(_WIN32))
#	define	___sync_val_compare_and_swap(dest,comp,valset) InterlockedCompareExchange(dest,valset,comp)

#	define __sync_add_and_fetch(dest,inc)	InterlockedIncrement(dest)

#	define __sync_sub_and_fetch(dest,inc)	InterlockedDecrement(dest)

#	define ___sync_test_and_set(dest,val)	InterlockedExchange(dest,val)

#else
#	define ___sync_test_and_set(dest,val) \
								{ long ___ostas; do { ___ostas=*dest; } while (__sync_val_compare_and_swap(dest, ___ostas, val) != ___ostas); }

#	ifndef ANDROID
#		define	___sync_val_compare_and_swap(dest,comp,valset) \
											__sync_val_compare_and_swap(dest,comp,valset)
#	endif
#endif

#if (defined(_WIN32) && !defined(CLI_CPP))
#	define HLIB								HMODULE
#else
#	define HLIB								void *
#endif

#include "Interop/Time.Val.h"

#	define OBJIDType          				int
#	define OBJIDTypeV          				volatile unsigned int


#if (defined(_WIN32) && !defined(CLI_CPP))
#	define	IWinRelease(IInst)				if (IInst) {IInst->Release(); IInst = 0;}

#	ifdef WINDOWS_PHONE
#		include "minwindef.h"
#		include "wtypes.h"
#		define COBSTR           			LPCWSTR
#		define WNDHANDLE           			void *
#	else /// <- WINDOWS_PHONE
#		ifndef CLI_CPP
#			include "windows.h"
#		endif
#		define COBSTR           			const char *
#		define WNDHANDLE           			HWND
#	endif /// <- WINDOWS_PHONE

#	define LONGSYNC           				unsigned long volatile
#	define LONGSYNCNV          				unsigned long

#   ifdef USE_LOCKFREE_SOCKET_ACCESS
#       define SOCKETSYNC           		long volatile
#       define SOCKETSYNCNV           		long
#   else
#       define SOCKETSYNC           		int
#       define SOCKETSYNCNV           		int
#   endif
#	define LONGSYNCNV          				unsigned long

/// Note: EBOOL represents platform specific boolean type. 
/// Since Objective-C BOOL conflicts with win32 BOOL, we introduce EBOOL to resolve name conflicts
#	ifdef WINDOWS_PHONE
#		define EBOOL						bool
#	else
#		define EBOOL						BOOL
#	endif

#	define EVAL_TO_STRING(s)				#s
#	define EXP_TO_STRING(s)					EVAL_TO_STRING(s)
// __FILE__ ":" EXP_TO_STRING(__LINE__)

#	define WARNING(msg) 					__pragma(message(__FILE__ "(" EXP_TO_STRING(__LINE__) ") : " #msg  ))
#	define INCLINEFUNC						inline 
											
#else

# ifdef CLI_CPP
#	define LONGSYNCNV          				__int64 
#	define LONGSYNC           				__int64

#   ifdef USE_LOCKFREE_SOCKET_ACCESS
#       define SOCKETSYNC           		__int64
#   else
#       define SOCKETSYNC           			int
#   endif

#	define WNDHANDLE           				IntPtr
#else
#	define LONGSYNCNV          				long
#	define LONGSYNC           				long

#   ifdef USE_LOCKFREE_SOCKET_ACCESS
#       define SOCKETSYNC           		long
#       define SOCKETSYNCNV           		SOCKETSYNC
#   else
#       define SOCKETSYNC           		int
#       define SOCKETSYNCNV           		SOCKETSYNC
#   endif

#	define WNDHANDLE           				void *
#endif

/// NOT _WIN32
#	define COBSTR           				const char *


#	ifdef CLI_CPP
#		define HMODULE						HINSTANCE
#	else
#		define HMODULE						void *
#	endif

#	ifdef __APPLE____
#		define WARNING(msg) 				_Pragma("warning(__FILE__ \"(\" EXP_TO_STRING(__LINE__) \") : \" #msg)")
#	else
#	define WARNING(msg)
#	endif

#	define INCLINEFUNC
//#define	__forceinline					inline
#	define	__forceinline

#	if (!defined(TCHAR) && !defined(CLI_CPP))
#		define TCHAR						char
#	endif

#	if defined(ANDROID) || !defined(__APPLE__)
#		define EBOOL						jboolean
#		define BSTR							jstring
#	else /// !ANDROID

/// Use bool as default for BOOL otherwise
#	define EBOOL							bool
#	define BSTR								const char *
#	endif  /// end-ANDROID

#	endif

#	ifdef CLI_CPP
#		ifdef EBOOL
#			undef EBOOL
#		endif
#		define EBOOL						bool
#	endif


// Not sure whether these are neccessary

#	if !defined(_WIN32)
#		define HANDLE						void *
#		define TCHAR						char

// for now required for DeviceDisplay.h
#		define CALLBACK
//#	define BOOLEAN							unsigned char
//#	define GetEncoderClsid(a,b)


#	define vsnprintf_s(dest,destSize,maxChar,format,vlist)		vsnprintf ( dest, destSize, format, vlist )
#	define sscanf_s(...)					sscanf(__VA_ARGS__)
#	define strtok_s(buf,delim,context)		strtok_r(buf,delim,context)
#	define localtime_s(a,b)					localtime_r(b,a)

#   ifdef __APPLE__
#       define Sleep(ms)					usleep(ms * 1000)
#   else
#       define Sleep(ms)					usleep(ms * 1000)
#   endif

#	endif // _WIN32   



// mathematical C++11 functions reverts to C99 functions for windows
#	if defined(_WIN32)

#		if _MSC_VER < 1700 
#			define fmax(a,b)		max(a,b)
#			define fmin(a,b)		min(a,b)
#		endif
#		if _MSC_VER < 1900 
#			define snprintf(...)	_snprintf(__VA_ARGS__)
#		endif

#	endif	/// end-_WIN32


#if defined(CLI_CPP)
	using namespace System;
	using namespace System::Runtime::InteropServices;
	using namespace System::Text;

#	define INTERNAL							internal
#	define free_m(m)
#	define free_n(m)						
#	define free_plt(m)                      if (m != nill) { environs::API::FreeNativeMemoryN(m); m = nill; }

#	define CLASS							ref class
#	define PUBLIC_CLASS						public ref class
#	define PUBLIC_STRUCT					public value struct
#	define STRUCT							value struct
#	define CLI_ABSTRACT						abstract
#	define c_const						
#	define DERIVE_c_only(v)					
#	define DERIVE_cli_only(v)				, v	
#	define OBJ_ptr							^
#	define cli_OBJ_ptr                      ^
#	define OBJ_p_cli(v,m)					v->m					
#	define c_OBJ_ptr				
#	define OBJ_ref							%
#	define cli_ref							%
#	define c_ref				
#	define CLI_OVERRIDE						override
#	define EPSPACE                          environs::
#	define CLIBSPACE						

#	define StringToCChar(s)					( const char* ) ( Marshal::StringToHGlobalAnsi ( s ) ).ToPointer ()
#	define DisposePlatCChar(v)				Marshal::FreeHGlobal ( IntPtr ( (void *) v ) )
#	define CCharToString(s)					Marshal::PtrToStringAnsi ( IntPtr ( (void *) (s) ) )

#	define STRING_T							System::String ^
#	define STRING_get(s)					s
#	define STRING_length(s)					s->Length
#	define STRING_empty(s)					System::String::IsNullOrEmpty(s)
#	define STRING_get_cstr(s)				StringToCChar ( s )

#	define ToPlatPointer(v)					Marshal::StringToHGlobalAnsi ( v ).ToPointer ()
#	define DisposePlatPointer(v)			Marshal::FreeHGlobal ( IntPtr ( v ) )
#	define Addr_ptr							System::IntPtr ^
#	define Addr_obj							System::IntPtr
#	define Addr_pvalue(v)					v->ToPointer()
#	define Addr_pvalued(v)					v.ToPointer()
#	define Addr_of(v)						(%v)
#	define c_Addr_of(v)                     v
#	define Addr_value(v)					v->ToInt32()
#	define CLI_NO_STATIC					

#	define CString_ptr						System::String ^	
#	define CString_obj						System::String	
#	define CString_ptr_empty(s)				STRING_empty(s)
#	define CVString_ptr						System::String ^	
#	define String_ptr						System::String ^	
#	define String_ptr_reset(s)				s = nill;
#	define CString_length(l)				l->Length
#	define CString_compare(l,r,s)			System::String::Compare ( l, r )
#	define CString_copy(l,s,r)				l = r
#	define CString_contains(s,c)			s->Contains ( c )
#	define CString_get_cstr(s)				s

#	define UCharArray_ptr					cli::array<unsigned char> ^
#	define PIN_PTR(p,t,n)					pin_ptr<t> p = &n [ 0 ]

#	define new__obj(type)					gcnew type()
#	define delete__obj(o)					o = nill		

// using namespace System::Runtime::InteropServices;
#	define CLI_INC							[System::Security::SuppressUnmanagedCodeSecurityAttribute ()] \
											[DllImport ( "Environs.dll", CallingConvention = CallingConvention::Cdecl )]
#	define CLI_INCM(m)						[System::Security::SuppressUnmanagedCodeSecurityAttribute ()] \
											[DllImport ( "Environs.dll", CallingConvention = CallingConvention::Cdecl, m )]
#	define CLI_OVERRIDE						override
#	define CLI_VIRTUAL						virtual

#	define TCHAR							char
#	define nill							nullptr
#   define C_Only(v)                        
#   define Cli_Only(v)                      v

#	define ENVOBSERVER(t1,t2)				event t2 ^
#	define CPP_CLI(t1,t2)					t2

#	define INIT_to_false_in_cli				= false
#	define INIT_to_EXP_in_cli(exp)			= exp
#	define EnvironsAPI(i)					environs::Environs::instancesAPI [ i ]

#   define MAKE_FRIEND_CLASS(c)
#	define DeviceInstancePropertyNotify(i,e)	OnPropertyChanged ( i, e )
//#	define DeviceInstanceNotify(i,f)		

#else

#	define INTERNAL							private
#	define free_m(m)						if (m != nill) { free(m); m = nill; }
#	define free_n(m)						if (m != nill) { free(m); }
#	define free_plt(m)                      free_m(m)
#	define CLASS							class
#	define PUBLIC_CLASS						class
#	define PUBLIC_STRUCT					struct
#	define STRUCT							struct
#	define CLI_ABSTRACT					
#	define c_const                          const
#	define DERIVE_c_only(v)					: public v
#	define DERIVE_cli_only(v)				
#	define OBJ_ptr							*
#	define c_OBJ_ptr                        *
#	define cli_OBJ_ptr					
#	define OBJ_p_cli(v,m)					v.m	
#	define OBJ_ref							&
#	define cli_ref							
#	define c_ref                            &
#	define CLI_OVERRIDE					
#	define EPSPACE					
#	define CLIBSPACE						environs::lib::

#	define CString_ptr						const char *	
#	define CString_obj						const char *
#	define CString_ptr_empty(s)				!(*s)
#	define CVString_ptr						const void *
#	define CString_length(l)				strlen ( l )
#	define CString_compare(l,r,s)			strncmp ( l, r, s )	
#	define CString_copy(l,s,r)				strlcpy ( l, r, s )	
#	define CString_contains(s,c)			strstr ( s, c )
#	define CString_get_cstr(s)				s.c_str ()

#	define STRING_T							std::string
#	define STRING_empty(s)					s.empty ()
#	define STRING_length(s)					s.length ()
#	define STRING_get(s)					s.c_str ()	
#	define STRING_get_cstr(s)				s.c_str ()	

#	define ToPlatPointer(v)					( (void *) v )
#	define DisposePlatPointer(v)			
#	define Addr_ptr							void *
#	define Addr_obj							void *
#	define Addr_pvalue(v)					v
#	define Addr_pvalued(v)					v
#	define Addr_of(v)						(&v)
#	define c_Addr_of(v)                     (&v)
#	define Addr_value(v)					((int) (size_t) v)
#	define CLI_NO_STATIC					static
#	define String_ptr						char *
#	define String_ptr_reset(s)				if (s != nill) { free(s); s = nill; }
#	define UCharArray_ptr					unsigned char *
#	define PIN_PTR(p,t,n)					t * p = n

#	define new__obj(type)					new type()	
#	define delete__obj(o)					if (o != nill)  { delete o; o = nill; }

#	define CLI_INC						
#	define CLI_INCM(m)			
#	define CLI_OVERRIDE						
#	define CLI_VIRTUAL						
#	define nill							NULL

#	define StringToCChar(s)					s
#	define DisposePlatCChar(v)				
#	define CCharToString(s)					s

#   define C_Only(v)                        v
#   define Cli_Only(v)                      

#	define ENVOBSERVER(t1,t2)				vct ( t1 * )
#	define CPP_CLI(t1,t2)					t1

#	define INIT_to_false_in_cli				
#	define INIT_to_EXP_in_cli(exp)			
#	define EnvironsAPI(i)					environs::lib::Environs::instancesAPI [ i ]

#   define MAKE_FRIEND_CLASS(c)            friend class c
#	define DeviceInstancePropertyNotify(i,e)		
#	define BuildDeviceDisplayProps(i,f)		i
#	define ParsePortalInfo(i)				i

#endif


#endif // INCLUDE_HCM_ENVIRONS_INTEROP_H

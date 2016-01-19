/**
 * Mediator base functionality
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
#ifndef INCLUDE_HCM_ENVIRONS_MEDIATOR_H
#define INCLUDE_HCM_ENVIRONS_MEDIATOR_H


#ifdef MEDIATORDAEMON
#   define  DAEMONEXP(exp)                      exp
#   define  CLIENTEXP(exp)

#   define MAX_SPARE_SOCKETS_IN_QUEUE			20
#   define MAX_SPARE_SOCKETS_IN_QUEUE_CHECK     26

#	define USE_LOCKFREE_SOCKET_ACCESS
#else
#   define  DAEMONEXP(exp)
#   define  CLIENTEXP(exp)                      exp

#   define MAX_SPARE_SOCKETS_IN_QUEUE			50
#   define MAX_SPARE_SOCKETS_IN_QUEUE_CHECK     60
#endif

//#define MEDIATOR_LIMIT_STUNT_REG_REQUESTS
//#define MEDIATOR_USE_TCP_NODELAY
//#define MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_SERVER
//#define MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_CLIENT
//#define USE_NONBLOCK_CLIENT_SOCKET

#include "Interop/Threads.h"
#include "Interop/Sock.h"
#include "Device.Info.h"
#include "Environs.Crypt.h"
#include "Interop/Smart.Pointer.h"


#define DEFAULT_MEDIATOR_PORT					5898
#define DEFAULT_BROADCAST_PORT					5899
#define	BUFFERSIZE								1024
#define MESSAGE_BUFFER_SIZE						512
#define	IP_MAX_SIZE								22

#define MEDIATOR_ANONYMOUS_USER					"anonymous"
#define MEDIATOR_ANONYMOUS_PASSWORD				"secret"

#define	MAX_MEDIATOR_PORTS						4	// We allow max. 4 ports for connections to the mediator

#define MEDIATOR_MESSAGE_UNIT_ALIGN				4
#define MEDIATOR_MESSAGE_LENGTH_FLAG_PAD		0x10000000

#define MEDIATOR_BROADCAST						"E.Mediator"
#define MEDIATOR_BROADCAST_DEVICE				"E.D."
#define MEDIATOR_SPARE_SOCKET_REQ				"E.DS"
#define MEDIATOR_BROADCAST_DEVICEINFO			"E.D.Client"
#define MEDIATOR_BROADCAST_GREET				"E.D.Helo"
#define MEDIATOR_BROADCAST_BYEBYE				"E.D.Bye"
#define MEDIATOR_PROTOCOL_VERSION_MIN			'2'
#define MEDIATOR_PROTOCOL_VERSION				'4'

#define	MEDIATOR_CMD_SET						's'
#define	MEDIATOR_CMD_GET						'g'
#define	MEDIATOR_CMD_GET_DEVICES				'd'
#define MEDIATOR_CMD_GET_DEVICES_LEN			16
#define MEDIATOR_CMD_GET_DEVICES_RESP_LEN		8
#define MEDIATOR_CMD_GET_DEVICES_COUNT_LEN		12
#define MEDIATOR_CMD_GET_DEVICES_COUNT_RESP_LEN	12
#define	MEDIATOR_CMD_HEARTBEAT					'h'
#define	MEDIATOR_CMD_GETALL						'a'
#define	MEDIATOR_CMD_SET_FILTERMODE				'f'
#define	MEDIATOR_CMD_SET_SOCKET_BUFFERS			'b'
#define	MEDIATOR_CMD_QUIT						'q'
#define	MEDIATOR_CMD_NATSTAT					'n'
#define	MEDIATOR_CMD_NOTIFICATION_SUBSCRIBE		'N'
#define	MEDIATOR_CMD_SHORT_MESSAGE				'm'
#define	MEDIATOR_CMD_MESSAGE_SUBSCRIBE          'M'
#define	MEDIATOR_CMD_REQ_SPARE_ID				'r'
#define	MEDIATOR_SRV_CMD_ALIVE_REQUEST			'A'
#define	MEDIATOR_SRV_CMD_SESSION_RETRY			'R'
#define	MEDIATOR_SRV_CMD_SESSION_LOCKED			'L'
#define	MEDIATOR_CMD_STUNT						'x'
#define	MEDIATOR_CMD_STUN						'y'
#define	MEDIATOR_CMD_HELP_TLS_GEN				't'
#define	MEDIATOR_CMD_SESSION_ASSIGN				's'
#define	MEDIATOR_CMD_AUTHTOKEN_ASSIGN			'a'
#define	MEDIATOR_CMD_MEDIATOR_NOTIFY			'i'
#define	MEDIATOR_CMD_MEDIATOR_NOTIFY_SIZE		16
#define	MEDIATOR_OPT_NULL						';'
#define	MEDIATOR_OPT_BLANK						'.'
#define	MEDIATOR_OPT_SIGNED_CERT_RESPONSE		's'
#define	MEDIATOR_CMD_DEVICE_LIST_QUERY			'd'
#define	MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT	'c'
#define	MEDIATOR_CMD_DEVICE_LIST_QUERY_RESPONSE	'l'
#define	MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR	'e'
#define	MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID		'i'

#define	MEDIATOR_CMD_GET_VERSION				'v'

#define MEDIATOR_STUNT_REQ_SIZE					12
#define MEDIATOR_STUNT_REQ_EXT_SIZE             76
#define MEDIATOR_STUNT_CHANNEL_VERSATILE		';'
#define MEDIATOR_STUNT_CHANNEL_MAIN				':'
#define MEDIATOR_STUNT_CHANNEL_BULK				'.'
#define MEDIATOR_STUNT_CHANNEL_PORTAL			','
#define MEDIATOR_STUNT_RESP_SIZE				28
#define MEDIATOR_STUNT_ACK_SIZE					12
#define MEDIATOR_STUNT_ACK_EXT_SIZE				20
#define MEDIATOR_STUN_REQ_SIZE					16
#define MEDIATOR_STUN_RESP_SIZE					20

#define MEDIATOR_NAT_REQ_SIZE					12
#define MEDIATOR_MSG_VERSION_SIZE				16
#define MEDIATOR_MSG_SOCKET_BUFFERS_SIZE		16

#define MEDIATOR_DEVICE_RELOAD					-1
#define MEDIATOR_DEVICE_CHANGE_NEARBY			1
#define MEDIATOR_DEVICE_CHANGE_MEDIATOR			2


const unsigned int		maxSpareSocketAlive	= 1000 * 60 * 5; // 5 min. (in ms)


namespace environs	/// Namespace: environs ->
{
	extern unsigned int		primaryInterface;

	typedef struct NetPack
	{
		unsigned int		ip;
		unsigned int		bcast;
		unsigned int		mask;
		unsigned int		gw;
		NetPack			*	next;
	}
	NetPack;

	struct DeviceInstanceNode;

	typedef struct ThreadInstance
	{
		void				*	daemon;

        int						deviceID;
        
        DAEMONEXP ( pthread_t   threadID );
        
        CLIENTEXP ( ThreadSync  listener; )
        
		pthread_mutex_t			lock;
		unsigned int			version;
		long long				sessionID;

		INTEROPTIMEVAL			aliveLast;
        
        SOCKETSYNC				socket;
		unsigned short			port;
		unsigned short			portUdp;

		struct 	sockaddr_in		addr;

        INTEROPTIMEVAL			connectTime;
        
        CLIENTEXP ( pthread_mutex_t spareSocketLock; )

        SOCKETSYNC				spareSocket;
        
		unsigned short			sparePort;
		short                   filterMode;
		
		int						encrypt;
		int						authenticated;
		AESContext				aes;

        int                     authLevel;
		bool					createAuthToken;
        char					uid [ MAX_NAMEPROPERTY * 6 ];
        char                    ips [ 20 ];

#ifdef __cplusplus        
        DAEMONEXP ( bool init );
        DAEMONEXP ( bool subscribedToNotifications );
        DAEMONEXP ( bool subscribedToMessages );

		sp ( ThreadInstance )	  clientSP;
		sp ( DeviceInstanceNode ) deviceSP;

        DAEMONEXP ( ~ThreadInstance () );
        
		int							spareSocketsCount;
        int							spareSockets [ MAX_SPARE_SOCKETS_IN_QUEUE_CHECK + 1 ];
        INTEROPTIMEVAL              spareSocketTime;
        
#ifdef MEDIATORDAEMON
        volatile ThreadInstance  *	stuntTarget;
        
#ifdef MEDIATOR_LIMIT_STUNT_REG_REQUESTS
        INTEROPTIMEVAL              stuntLastSend;
        
        INTEROPTIMEVAL              stunLastSend;
#endif
        
        pthread_mutex_t             spareSocketsLock;
        
		bool Init ();
        bool Lock ( const char * func );
        bool Unlock ( const char * func );
#endif
        
#endif
	}
	ThreadInstance;

    extern void LimitSpareSocketsCount ( ThreadInstance * client );
    extern int GetSpareSocket ( ThreadInstance * inst );

	class ILock
	{
		bool init;

	public:
		pthread_mutex_t		lock;

		bool Init ();
		bool Lock ( const char * func );
		bool Unlock ( const char * func );
		ILock ();
		~ILock ();
	};


	class ApplicationDevices : public ILock
	{
	public:
		ApplicationDevices ();
		~ApplicationDevices ();

		unsigned int			id;
		unsigned int			areaId;
		int						count;
		long					access;
		
		long					latestAssignedID;

		DeviceInstanceNode	*	devices;

		bool					deviceCacheDirty;
		int						deviceCacheCount;
		DeviceInstanceNode	*	devicesCache;
	};

    
    // "int11 1s areaNameMAX_PROP 1s appNameMAX_PROP 1e"
#define MAX_DEVICE_INSTANCE_KEY_LENGTH      (11 + MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 9)
    
    CLIENTEXP ( class DeviceController; )
    
    
	typedef struct DeviceInstanceNode
	{
		lib::DeviceInfo info;
        
        DAEMONEXP ( sp ( ApplicationDevices ) rootSP; )
        DAEMONEXP ( sp ( ThreadInstance )     clientSP; )
        
        // "int11 1s areaNameMAX_PROP 1s appNameMAX_PROP 1e"
        CLIENTEXP ( char					  key [MAX_DEVICE_INSTANCE_KEY_LENGTH]; );

		char					  userName	[ MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 1 ]; // 31

        // Links used by Mediator base layer
		struct DeviceInstanceNode	*	next;  // 4
		struct DeviceInstanceNode	*	prev;  // 4
		
        CLIENTEXP ( int					hEnvirons; )
		        
#ifdef __cplusplus        
        sp ( DeviceInstanceNode )		baseSP;         // SP used by Mediator base layer
        
        CLIENTEXP ( sp ( DeviceInstanceNode ) mapSP; )  // SP used by Mediator client layer
        
        CLIENTEXP ( wp ( DeviceController ) deviceSP; )  // WP used by Mediator client layer
        
        DeviceInstanceNode ();
        
		~DeviceInstanceNode ();
#endif
        
	}
	DeviceInstanceNode;

    
	typedef struct DeviceInfoNode
	{
		lib::DeviceInfo info;

        // Links used by Mediator base layer
		struct DeviceInfoNode	*	next;  // 4        
	}
	DeviceInfoNode;


	typedef struct _MediatorThreadInstance
	{
		ThreadInstance			instance;
		int						socketUdp;
		pthread_t				threadIDUdp;
		pthread_t				threadIDWatchdog;
	}
	MediatorThreadInstance;


	typedef struct _MediatorConnection
	{
		ThreadInstance			instance;

		char				*	buffer;
		pthread_mutex_t			send_mutex;

		pthread_mutex_t			rec_mutex;
		pthread_cond_t			rec_signal;
		char				*	responseBuffer;
        bool                    longReceive;

		INTEROPTIMEVAL			lastSpareSocketRenewal;
        LONGSYNC                renewerAccess;
        LONGSYNC                renewerQueue;
        
        Instance            *   env;
	}
	MediatorConnection;


	typedef struct MediatorInstance
	{
		unsigned int			ip;
		unsigned short			port;
        
		bool					enabled;
		bool					listening;
		MediatorConnection		connection;
        void                *   mediatorObject;
		MediatorInstance	*	next;
	}
	MediatorInstance;

	typedef void ( *MediatorCallback )(void *);

#if (!defined(MEDIATORDAEMON) && defined(__cplusplus))
	class Instance;
#endif

	/**
	*	Mediator base functionality
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	02/01/13
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class Mediator
	{
	public:
		Mediator ( );
		virtual ~Mediator ( );

		bool					Init ();
		bool					Start ();
		bool					IsStarted ();

		void					StopMediators ();

		virtual void			BuildBroadcastMessage () = 0;
		bool					SendBroadcast ();

		bool					AddMediator ( unsigned int ip, unsigned short port );
        void					BroadcastByeBye ();
        
        static bool				LoadNetworks ();
        static unsigned int		GetLocalIP ();
        static unsigned int		GetLocalSN ();

        static bool				InitClass ();
        static void				DisposeClass ();
        
        static bool             IsAnonymousUser ( const char * user );

		static int              localNetsSize;

	protected:
        bool					allocated;
        static bool				allocatedClass;

#if (!defined(MEDIATORDAEMON) && defined(__cplusplus))
		sp ( Instance )			envSP;
		Instance			*	env;
#endif
        
		bool					isRunning;
		char			*		certificate;

		static NetPack          localNets;
        static pthread_mutex_t  localNetsLock;
        
		pthread_mutex_t			mediatorLock;
		MediatorInstance		mediator;

		pthread_mutex_t			devicesLock;

		unsigned int			broadcastMessageLen;
		char					broadcastMessage [MEDIATOR_BROADCAST_DESC_START + ((MAX_NAMEPROPERTY + 2) * 6) + 4]; // 4; 12; 4; 4; 2; 2; => 24 byte; max. 50 byte for areaName

		unsigned long long		lastGreetUpdate;

		int						broadcastSocketID;
		pthread_t				broadcastThreadID;

		bool					IsLocalIP ( unsigned int ip );
        static void				VerifySockets ( ThreadInstance * inst, bool waitThread );
        
        static bool				IsSocketAlive ( SOCKETSYNC &sock );
        
		virtual void			OnStarted ( );

		void					VanishedDeviceWatcher ();

		virtual sp ( ApplicationDevices ) GetDeviceList ( char * areaName, char * appName, pthread_mutex_t ** mutex, int ** pDevicesAvailable, DeviceInstanceNode ** &list ) = 0;

		virtual void			RemoveDevice ( unsigned int ip, char * msg ) {};
		virtual void			RemoveDevice ( DeviceInstanceNode * device, bool useLock = true ) = 0;

#ifdef __cplusplus
		sp ( DeviceInstanceNode ) UpdateDevices ( unsigned int ip, char * msg, char ** uid, bool * created, char isBroadcast = 0 );

		virtual void			UpdateDeviceInstance ( sp ( DeviceInstanceNode ) device, bool added, bool changed ) = 0;
#endif
		virtual void			ReleaseDevices ( ) = 0;

		//virtual void			NotifyClients ( unsigned int notify, const char * areaName, const char * appName, int deviceID ) {};
		
		MediatorInstance *		IsKnownMediator ( unsigned int ip, unsigned short port );
		MediatorInstance *		AddMediator ( MediatorInstance * med );
		virtual bool			RegisterAtMediator ( MediatorInstance * med );
		bool					RemoveMediator ( unsigned int ip );

		static void *			BroadcastThreadStarter ( void *arg );
		virtual void *			BroadcastThread ( ) = 0;

		void					Dispose ( );
		void					ReleaseMediators ( );
		void					ReleaseMediator ( MediatorInstance * med );
        bool					ReleaseThreads ( );
        
        static void				ReleaseNetworks ( );

	private:
        static void				AddNetwork ( unsigned int ip, unsigned int bcast, unsigned int netmask, unsigned int gw );
        static unsigned int		GetBroadcast ( unsigned int ip, unsigned int netmask );

		virtual void 			DevicesHasChanged ( int type );
	};

	/// Attention: 64 bit value must be aligned with 8 bytes
	typedef struct _MediatorMsg
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		union {
			long long		sessionID;

			struct {
				unsigned int	msgID;

				union {
					unsigned int	id;
					int				notifyDeviceID;
					char			msg;
				}
				data;
			} id2;
		} ids;

	}
	MediatorMsg;


	typedef struct _MediatorReqMsg
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
        
		/** The machine name of the device */
		char			deviceUID [MAX_NAMEPROPERTY * 6]; // 31
	}
	MediatorReqMsg;


	typedef struct _MediatorGetPacket
	{
		unsigned int	size;
		char			version;
		char			cmd;
		char			opt0;
		char			opt1;
	}
	MediatorGetPacket;


	typedef struct _MediatorNotify
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		unsigned int	msgID;
		int				notifyDeviceID;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	MediatorNotify;


	typedef struct _MediatorQueryMsg
	{
		unsigned int	size;
		char			cmdVersion;
		char			cmd1;
		char			opt0;
		char			opt1;

		unsigned int	msgID;
		unsigned int	startIndex;
		int				deviceID;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	MediatorQueryMsg;


	typedef struct _MediatorQueryResponse
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		lib::DeviceHeader	deviceHead;
	}
	MediatorQueryResponse;


	typedef struct _SpareSockRegPack
	{
		unsigned int	alignPad;
		unsigned int	sizeEncrypted;
		unsigned int	sizeReq;

		char			ident [4];
		long long		sessionID;
		unsigned int	sizePayload;
		char			payload;
	}
	SpareSockRegPack;


	typedef struct _SpareSockDecPack
	{
		unsigned int	sizeReq;

		char			ident [4];
		long long		sessionID;
		unsigned int	sizePayload;
		char			payload;
	}
	SpareSockDecPack;


	typedef struct _STUNTReqPacket
	{
		unsigned int	size;
		char			version;
		char			ident [2];
		char			channel;

		int				deviceID;
        
		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31
        
		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31

		char			pad [2];
	}
	STUNTReqPacket;


	typedef struct _STUNTRespPacket
	{
		unsigned int	size;
		char			respCode;
		char			ident [2];
		char			channel;

        unsigned short	porti;
        unsigned short	porte;
		unsigned int	ip; // 12
		unsigned int	ipe; // 16
	}
	STUNTRespPacket;

	typedef struct _STUNTRespReqPacket
	{
		unsigned int	size;
		//char			respCode;
		char			ident [3];
		char			channel;

		int				deviceID;
		unsigned int	ip;
		unsigned int	ipe;
        unsigned short	porti;
        unsigned short	porte;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
		unsigned int	pad0;
	}
	STUNTRespReqPacket;


	typedef struct _STUNReqPacket
	{
		char			ident [4];

		unsigned int	sourceID;
		unsigned int	destID;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31

		/** The area name of the application environment. */
		char			areaNameSrc [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appNameSrc [MAX_NAMEPROPERTY + 1]; // 31
	}
	STUNReqPacket;


	typedef struct _STUNReqReqPacket
	{
		unsigned int	size;
		char			ident [4];

		int				deviceID;
        unsigned int	IPi;
        unsigned int	IPe;
        unsigned short	Porti;
        unsigned short	Porte;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	STUNReqReqPacket;

	
	typedef struct _STUNTRegReqPacket
	{
		unsigned int	size;
		char			version;
		char			ident [3];

		unsigned int	notify;
		unsigned int	pad0;
	}
	STUNTRegReqPacket;


	typedef struct _UdpHelloPacket
	{
		char			ident [3];
		char			version;

		int				deviceID;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	UdpHelloPacket;


	typedef struct _UdpEncHelloPacket
	{
		unsigned int	size;
		unsigned int	alignPad;
		long long		sessionID;
		char			aes;
	}
	UdpEncHelloPacket;


	typedef struct _ShortMsgPacket
	{
		unsigned int	size;
		char			version;
		char			ident [3];

		int				deviceID;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31

		char			msg;
	}
	ShortMsgPacket;


	typedef struct _ShortMsg
	{
		char			version;
		char			deviceType;
		char			ident [2];
		int				deviceID;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31

		char			text;
	}
	ShortMsg;


} /* namepace Environs */


#endif // INCLUDE_HCM_ENVIRONS_MEDIATOR_H

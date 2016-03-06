### v 0.9.8.0

---------------------------

* Updated: Linger zero for socket closer



### v 0.9.7.1

---------------------------

* Added: Mediator protocol version to broadcast identifier
* Updated: Mediator STUN protocol with flat and compressed apparea
* Updated: GetDeviceList from Mediator to V5 protocol
* Changed: Broadcast and Mediator protocol version to 5
* Added: Populate platform object activation flag
* Added: Type MediatorStatusMsgExt for device updates
* Added: Broadcast deviceflags update
* Updated: Mediator server with deviceflags update handler


### v 0.9.7.0

---------------------------

* Disabled: Threadname assignment
* Fixed: Bug in MediatorDaemon and ip bann


### v 0.9.6.8

---------------------------

* Updated: Mediator daemon check for deviceName for busy slot detection
* Updated: Mediator daemons client thread when a identity is already registered by someone
* Updated: Thread disposal in Threads.cpp
* Updated: Stun/Stunt min wait time between notifications


### v 0.9.6.5

---------------------------

* Updated: Mediator daemon socket checks


### v 0.9.6.4

---------------------------

* Updated: socket calls in mediator daemon
* Updated: Mediator service stun register request frequency
* Updated: Log output


### v 0.9.6.2

---------------------------

* Fixed: GetDeviceID from Mediator
* Fixed: Bug in Mediator layer if multiple (virtual) network interfaces are available or multi-homed hosts
* Updated: Log layer



### v 0.9.6 

---------------------------

* Updated: MediatorDaemon WatchDog client alive check
* Added: (Fix for mediator responses) Padding to DeviceInfo
* Added: Safe dispose of session clients in MediatorDaemon
* Fixed: Bug in MediatorDaemon where mssing areaMaps lead to failing of registration of new clients
* Updated: Use of lock callback in crypt layer (CryptoGetThreadID)
* Updated: Detection of natStat moved to registering at Mediator service
* Fixed: SecureChannelAuth in Mediator layer if message is transfered multipart
* Added: Use of lock callback in crypt layer
* Updated: MediatorDaemon protocol for shortmessages
* Added: ERR_remove_thread_state to DynLib.Crypto


### v 0.9.5.2

---------------------------

* Fixed: Network interface detection on windows platforms


### v 0.9.5

---------------------------

* Moved: Disable linger to closesocket
* Added: SetMessagesSubscription/GetMessagesSubscription to Mediator layer
* Added: SetMediatorNotificationSubscription/GetMediatorNotificationSubscription to Mediator layer
* Added: ERR_remove_state to cleanup crypto resources on error
* Added: ENABLE_DONT_LINGER_SOCKETS to native layer
* Added: Lock to spare sockets container
* Updated: ReplaceThreadSocket in MediatorDaemon
* Updated: Disposal of ThreadInstances on shutdown of MediatorDaemon
* Updated: Handling of spare sockets in MediatorDaemon
* Fixed: Detaching of ClientThreads for spare socket registrations
* Added: Check for outdated project values in MediatorDaemon (WatchDog)
* Added: Option s to toggle stdout
* Added: Fallback to LoadNetworks for Windows in Mediator base layer


### v 0.9.4

---------------------------

* Release: 0.9.4
* Updated: MediatorDaemon NotifyClients
* Updated: Environs crypt layer
* Added: Disable DISABLE_MEM_CHECKS to build opts
* Added: Toggle Acceptor of Mediator Daemon (j)
* Updated: Environs.Build.Opts.h
* Updated: Windows Mediator local network adapter detection


### v 0.9.3

---------------------------

* Release: 0.9.3
* Updated: Usage of snprintf (iOSX, Mediator)
* Replaced: sprintf_s with snprintf (_snprintf for vs < 2015)
* Removed: Mediator daemon support for protocol version <= 2
* Added: ApplicationDevices cache preparation for MD
* Updated: Added Mediator response timeout 3 min. (in case the provided response buffer is larger than 512 bytes)
* Added: ThreadSync class


* Release: 0.9.2
* Updated: Lock logs with threadID
* Enabled: Lockfree access to sockets
* Updated: Lock logging
* Reworked: DisposeThread / DetachThread
* Updated: LoadNetworks / ReleaseNetowrks in Mediator base layer
* Changed: Using strlcpy/strlcat instead of strcpy_s/strcat_s
* Reworked: LoadNetworks/AddNetwork in Mediator base layer


* Release: 0.9.1
* Updated: MD lockfree usage of sockets
* Added: MD logroll if file is larger than 100MB
* Updated: MediatorDaemon usage of SPs
* Added: MediatorDaemon reopen logfile regularly by watchdog
* Added: Mediator vs2013 solution
* Fixed: Missing Unlock in MediatorDaemon (HandleSTUNTRequest)
* Fixed: Bug in MediatorDaemon (HandleSTUNTRequest)
* Fixed: Uninitialized variable in MediatorDaemon
* Updated: MediatorDaemon Disposal of ClientThread resources
* Updated: MediatorDaemon to check connectTime within WatchDog
* Updated: MediatorDaemon with DeviceInfoNode
* Updated: Usage of deviceSP in MediatorDaemon
* Fixed: Bug in Lock/Access list of HandleShortMessage (MediatorDaemon)
* Updated: Disposal of MediatorDaemon SpareSocket reference in ClientInstance
* Added: Instance SP to Mediator
* Updated: Mediator client access while disposing
* Updated: Concurrent access to mediator (while disposing)
* Updated: AliveThread sending a helo to help vanished devices watcher (duration 120000 ms)
* Updated: Vanished devices watcher
* Added: Vanished devices watcher to base Mediator layer (for broadcast found devices)


* Release: 0.9.0
* Updated: Notification thread for device list changes


* Release: 0.8.8
* Updated: CI yml.
* Updated: Mediator Linux makefile (include paths).
* Fixed: Mediator Linux makefile to include current directory as last header search path.
* Fixed: Mediator OSX compile error due to missing openssl headers in OSX El Capitan.

* Release: 0.8.6
* Added: yml configuration for gitlab CI.
* Updated: Include files to latest naming conventions.

* Added: Anonymous logon to Mediator (reduced privileges to one application environment).
* Extended: User database and encryption.
* Updated: to version 0.8.0
* Changed: ProjectName to AreaName


### v 0.7.3

---------------------------

* Fixed: Compile error on Linux platforms.
* Updated: Release 0.7.3

* Fixed: Disconnects from Mediator due to inactivity of devices.
* Updated: to 0.7.0 (Preparing release)


### v 0.6.0

---------------------------

* Updated: Prebuilt binaries to 0.6.0
* Updated: Common to 0.6.0
* Reimported repository for 0.6.0 release



### v 0.4.3.5466 (release)

---------------------------

* Changed: Mediator shows version string (main.cpp) instead of building the string again
* Changed: Mediator saves bann list only if bann list has been changed in BannIPRemove()
* Changed: method in MediatorDaemon from BannIPClear to BannIPRemove
* Added: Mediator clears a bann on successful login
* Added: enum type Platforms
* Added: Version request/handling in Mediator layer (Service/Client). Client provides GetMediatorServiceVersion sending the messge MEDIATOR_CMD_GET_VERSION which the Service now understands.
* Added Environs.revision.h to Windows Mediator project
* Updated: Mediator handling of banned IPs
* Lowered: Mediator log messages for banned IP connects



### v 0.4.2.5445 (unreleased)

---------------------------

* Updated Mediator.OSX 
* Reduced log messages for mediator layers


### v 0.4.2

---------------------------

* Fixed: Mediator derives an AES context even if no mediator.conf can be read
* Added: Mediator banns an IP after bannAfterTries (default 3) unsuccessfull tries. Added config option CV: to configure this in mediator.conf
* Fixed: Bug in Mediator. Root cause due to notifyClients thread.
* Added: Mediator shows users when printing the database on console
* Fixed: Authentication bug with a Mediator service in common native layer.


### v 0.4.1 (rev 5417/5418)

---------------------------

* Initial merge from development branch to master branch
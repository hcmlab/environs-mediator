### v 0.8.0

---------------------------

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
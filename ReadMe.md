# Environs Mediator sources and binaries

----------------------------------------
*  Created on: 01.05.2015
*      Author: Chi-Tai Dang, dang@hcm-lab.de

The Mediator service is an optional service for Environs application environments that help spanning application environments across network boundaries.<br>
The Mediator service is designed as command line application and accepts keyboard commands through the command line. However, the service may also run as background daemon.



## Binaries and How-to build

----------------------------
* You may either use the precompiled binaries in the [Mediator](Mediator) folder or
* build the binaries yourself

### OSX/Linux
```
> cd Mediator
> ./build.sh
```

The build script performs a make clean && make. The resulting binaries are then called [Mediator.OSX](Mediator/Mediator.OSX) / Mediator.Linux.

### Windows

-------------
* open the Visual Studio 2010 solution [Mediator/Mediator.sln](Mediator/Mediator.sln)

* Build either debug or release version.



## Directory structure and contents

-----------------------------------

### /[Common](Common)

-----------
* Header files and tools that are commonly used by all platforms.


### /[Mediator](Mediator)

-------------
* Mediator source code and binaries for Windows 32/64 bit ([Mediator.x86.exe](Mediator/Mediator.x86.exe) / [Mediator.x64.exe](Mediator/Mediator.x64.exe)) and [Mediator.OSX](Mediator/Mediator.OSX).



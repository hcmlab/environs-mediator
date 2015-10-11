CC	= g++
# This must be set to the common code directory of Environs
INCS	= ../Common/
3RD	= ../3rd/

OUTF	= Mediator
ifeq ($(OS),Windows_NT)
    OUTF = Mediator.Win.exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
	OUTF = Mediator.Linux
    endif
    ifeq ($(UNAME_S),Darwin)
	OUTF = Mediator.OSX
    endif
endif

# For release builds add -DNDEBUG to suppress debug messages

CFLAGS	= -Wall -Wno-unused -DMEDIATORDAEMON -DLINUX -DMEDIATOR -g -I$(INCS) -I$(3RD) -I/opt/ssl/include/ -I/opt/local/include -I$(3RD)inc -Wno-deprecated
LDFLAGS	= -lm -lpthread -lcurses -lcurl -ldl -L/opt/local/lib 
#-lcrypto
OTHER_C_FLAGS = -isystem /opt/local/include

ifndef DEBUG
   CFLAGS += -Os -DNDEBUG
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
   CFLAGS += -Wno-unused-but-set-variable
endif

OBJ	= main.o stdafx.o Mediator.o Mediator.Daemon.o Threads.o Environs.Types.o Environs.Utils.o Environs.Crypt.o Export.o Dyn.Lib.Crypto.o


all: $(OBJ)
	$(CC) $(CFLAGS) -o $(OUTF) $(OBJ) $(LDFLAGS)

Mediator.o: $(INCS)Mediator.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

Threads.o: $(INCS)Interop/Threads.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

Environs.Types.o: $(INCS)Environs.Types.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

Environs.Utils.o: $(INCS)Environs.Utils.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
    
Environs.Crypt.o: $(INCS)Environs.Crypt.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
    
Export.o: $(INCS)Interop/Export.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
    
Dyn.Lib.Crypto.o: $(3RD)DynLib/Dyn.Lib.Crypto.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	rm *.o $(OUTF)

#prepare: 
#	@if [ -d "/opt/local/include/openssl" ]; then export CFLAGS += -I/opt/local/include/openssl; fi

	
CC	= g++
# This must be set to the common code directory of Environs
INCS	= ../Common/

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

CFLAGS	= -Os -DNDEBUG -Wall -DMEDIATORDAEMON -DLINUX -DMEDIATOR -I$(INCS) -I. -g -I/opt/ssl/include/  -I/opt/local/include -Wno-deprecated
LDFLAGS	= -lm -lpthread -lcurses -lcurl -L/opt/local/lib -lcrypto
OTHER_C_FLAGS = -isystem /opt/local/include


OBJ	= main.o stdafx.o Mediator.o Mediator.Daemon.o threads.o Environs.types.o Environs.utils.o Environs.crypt.o


all: $(OBJ)
	$(CC) $(CFLAGS) -o $(OUTF) $(OBJ) $(LDFLAGS)

Mediator.o: $(INCS)Mediator.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

threads.o: $(INCS)Interop/threads.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

Environs.types.o: $(INCS)Environs.types.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

Environs.utils.o: $(INCS)Environs.utils.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
    
Environs.crypt.o: $(INCS)Environs.crypt.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	rm *.o $(OUTF)

#prepare: 
#	@if [ -d "/opt/local/include/openssl" ]; then export CFLAGS += -I/opt/local/include/openssl; fi

	
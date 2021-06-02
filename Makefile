CC=gcc

CFILES=NfcFactoryTestApp.c

CFILES_DRV= tml_drv.c
CFILES_ALT= tml_alt.c

INCLUDES= -I/usr/include/ -I./

LIBS=

CFLAGS=-Wall $(INCLUDES)

drv:
ifeq ("$(DEBUG)","ON")
	$(CC) -o NfcFactoryTestApp $(CFLAGS) -DDEBUG $(CFILES) $(CFILES_DRV) $(LIBS)
else
	$(CC) -o NfcFactoryTestApp $(CFLAGS) $(CFILES) $(CFILES_DRV) $(LIBS)
endif

alt:
ifeq ("$(DEBUG)","ON")
	$(CC) -o NfcFactoryTestApp $(CFLAGS) -DDEBUG $(CFILES) $(CFILES_ALT) $(LIBS)
else
	$(CC) -o NfcFactoryTestApp $(CFLAGS) $(CFILES) $(CFILES_ALT) $(LIBS)
endif

clean:
	rm NfcFactoryTestApp

install:
	cp NfcFactoryTestApp /usr/local/bin/

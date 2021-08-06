CC=gcc

CFILES=NfcFactoryTestApp.c

CFILES_DRV= tml_drv.c
CFILES_I2C= tml_alt-i2c.c
CFILES_SPI= tml_alt-spi.c

INCLUDES= -I/usr/include/ -I./

LIBS=

CFLAGS=-Wall $(INCLUDES)

drv:
ifeq ("$(DEBUG)","ON")
	$(CC) -o NfcFactoryTestApp $(CFLAGS) -DDEBUG $(CFILES) $(CFILES_DRV) $(LIBS)
else
	$(CC) -o NfcFactoryTestApp $(CFLAGS) $(CFILES) $(CFILES_DRV) $(LIBS)
endif

alt-i2c:
ifeq ("$(DEBUG)","ON")
	$(CC) -o NfcFactoryTestApp $(CFLAGS) -DDEBUG $(CFILES) $(CFILES_I2C) $(LIBS)
else
	$(CC) -o NfcFactoryTestApp $(CFLAGS) $(CFILES) $(CFILES_I2C) $(LIBS)
endif

alt-spi:
ifeq ("$(DEBUG)","ON")
	$(CC) -o NfcFactoryTestApp $(CFLAGS) -DDEBUG $(CFILES) $(CFILES_SPI) $(LIBS)
else
	$(CC) -o NfcFactoryTestApp $(CFLAGS) $(CFILES) $(CFILES_SPI) $(LIBS)
endif

clean:
	rm NfcFactoryTestApp

install:
	cp NfcFactoryTestApp /usr/local/bin/

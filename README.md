linux_NfcFactoryTestApp
=======================
NFC Factory Test application for characterization of the PN71xx NFC Controller integration into linux based device.

Allows setting the NFC controller into either:
 - Constant RF emission mode (no modulation)
 - Functional mode (card detection)
 - PRBS (Pseudo Random Binary Sequence) mode (continuous modulation)
 - Standby mode (for power consumption measurment)
Additionnaly, the application allows to:
 - Dump all RF settings values
 - Set RF settings
 - Get NCI parameters value
 - Set NCI parameters value
 - Get proprietary parameters value
 - Set proprietary parameters value

To build the application: "$ make <option>" with <option>:
  - drv: application communicates with PN71xx through kernel driver (either nxpnfc or pn5xx_i2c)
  - alt-i2c: application communicates with PN71xx through alternate I2C solution (see tml_alt-i2c.c for connection definition)
  - alt-spi: application communicates with PN71xx through alternate SPI solution (see tml_alt-spi.c for connection definition)

Adding "DEBUG=ON" statement to the make command configure the application to display all NCI exchanges.

More information about the application can be found in AN13287 (http://www.nxp.com/doc/AN13287)

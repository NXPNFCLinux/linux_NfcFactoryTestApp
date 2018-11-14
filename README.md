linux_NfcFactoryTestApp
=======================
NFC Factory Test application for characterization of the PN71xx NFC Controller integration into linux based device.

Allows setting the NFC controller into either:
-	Constant RF emission mode (no modulation)
-	or PRBS (Pseudo Random Binary Sequence) mode (continuous modulation)
-	Standby mode

To build the application: $ make <option>
with <option>: 
  - pn5xx: application communicates with PN71xx through pn5xx_i2c kernel driver
  - alt: application communicates with PN71xx through alternate solution (see tml_alt.c for connection definition)
  - dummy: dummy example to implement your own communication channel to PN71xx

More information can be found in AN11697: http://www.nxp.com/documents/application_note/AN11697.pdf
  

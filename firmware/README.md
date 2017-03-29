# Firmware directory

This directory contains the firmware for Blodwen. More details, such as the
build process, can be found in the LaTeX documentation.

## common
This contains the common files used by both. The other directories contain links
to the files in here.
Before
the firmware can be compiled, the ```build``` script needs to be run.
This used to use the Haskell file ```regparse.hs``` to generate the files 
but now they're pretty much static, so we just copy ```regsauto.cpp```
into ```regsauto.ino```. 

## master
This contains the code for the master Arduino Uno board, which mediates
between the PC and motor controllers, runs the remote control system,
and monitors the temperature. As
such, it uses the OneWire library (already included in lib). I use
``ino`` to build this but ``platformio`` could be persuaded to do it.

## slave
This contains the code to generate the firmware for the motor controllers.
There are two variants, for Lift/Steer controllers and Drive/Drive controllers.
Which variant is built is determined by preprocessor constants - the 
```bdrive``` and ```blift``` scripts set the appropriate values
and run the build for each variant. Each controller must have a unique ID which must be
set in the EEPROM. Two other scripts, ``changeid`` and ``upload``, allow
the ID of a connected slave to the changed, and allow firmware to be
uploaded and the ID changed in a single step.

## temptests and test 
These contain test code used during development for testing the
OneWire interface to the temperature monitors and remote control
respectively.

## orig
This contains the original motor controller code as it was loaded
onto the motor controller units when they were delivered. It is
provided for reference but not used.

# Blodwen rover code and documentation
This repository contains the documentation and source code for the
Blodwen rover, also known as the AEPR (Aberystwyth Experimental
Planetary Rover).

The documentation is in the **doc** directory, in LaTeX and as
a prebuilt PDF. If you need to rebuild the docs, the SVGs will
need to be converted to PDFs of the same basename (e.g. 
```motors1.svg``` will need to be converted to ```motors1.pdf```).
I do this with an bash/Inkscape script, which is provided: 
```svgs2pdfs```.

The code consists of:
* **pc**: the Blodwen PC library which communicates with the rover chassis
hardware (i.e. the master Arduino): a script is provided to convert this into a tar archive for incorporation into other projects;
* **firmware**: the firmware for the master Arduino and the slave motor controllers. This is intended to be compiled with ```ino``` but can be readily modified;
* **roverScript**: the Angort scripting environment which nobody but Jim uses.

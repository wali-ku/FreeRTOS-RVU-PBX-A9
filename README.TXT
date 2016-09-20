This is the FreeRTOS port for the RealView-PBX-A9 which can be run in QEMU Virtual Machine.
The code included in this port has been copied from the following two sources:


Source-1:
================================================================================================
This is the FreeRTOS port for the Versatile Express using the CoreTile 9x4 Daughterboard
equipped with the ARM Cortex-A9MPCore Quad-Core CPU.

Authors: Dag Ågren, Åbo Akademi University, Finland
         Simon Holmbacka, Åbo Akademi University, Finland
 
This port has been developed as a part of the RECOMP project in Åbo Akademi University, Finland
http://research.it.abo.fi/
Contact email: sholmbac@abo.fi
=================================================================================================


Source-2:
=================================================================================================
FreeRTOS on ARM926ej-s CPU by Jernej Kovacic
https://github.com/jkovacic/FreeRTOS-GCC-ARM926ejs
=================================================================================================

The headers from respective authors have been left intact in the relevant source files.
This is a single core port which runs the multi-threaded freeRTOS demo on RealView-PBX-A9. It has
been verified to be working.
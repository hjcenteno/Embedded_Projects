# Embedded_Projects
This will be my embedded project portfolio

<u>*Board Used*</u>

[**Stm32 Nucleo-G474RE**](https://www.st.com/en/evaluation-tools/nucleo-g474re.html)

<u>*My Environment*</u>

- <b>OS</b> - Pop OS
- <b>IDE</b> - VSCode
***
<u>*Installation/Setup*</u>

Before running the cmake file, make sure you have arm_gcc, openocd.

*If using ubuntu/debian based distro*:
- run `sudo apt install gcc-arm-none-eabi`
- run `sudo apt install openocd`

*otherwise*,

Downloand ARM GNU toolchain from developer.arm.com
##
To create the linker and startup code for the nucleo-g474re
- run `cmake -P bootstrap.cmake` 

This will setup everything the makefile expects and installs:
- cmsis_device_g4
- cmsis_core
***
<u>*Make Commands*</u>

Embedded_Projects Makefile -- available targets
-  `make <project>` to build a project
-  `make flash-<project>` to flash it
-  `make clean [proj ...]` to remove the bin directory for the named projects. If no project was given, will remove each bin directory in all the project directories.
-  `make list` list out every project
-  `make project <name>` create a new project dir
-  `make lib <name>` compile the given lib source file for a syntax check, does not link
-  `make newlib <name>` create a new lib directory for thesource and header files
-  `make listlib` list every library available under the lib directory
-  `make HELP` shows what the make file can do
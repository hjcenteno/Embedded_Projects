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
- `make list` to see all the buildable projects.
- `make project {project_name}` to make a new project directory
- `make {project_name}` to build.
- `make flash-{project_name}` to flash.
- `make clean {project_name}` to clean the bin.
CC       := arm-none-eabi-gcc
OBJCOPY  := arm-none-eabi-objcopy
SIZE     := arm-none-eabi-size

MCU      := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# Point these at wherever you cloned the CMSIS repos (see earlier setup)
CMSIS_CORE   := $(HOME)/cmsis_core
CMSIS_DEVICE := $(HOME)/cmsis_device_g4

SRC_DIR       := src
HDR_DIR       := $(SRC_DIR)/headers
STARTUP_DIR   := ../startup
LINKER_SCRIPT := ../linker/STM32G474RE_FLASH.ld
BIN_DIR       := bin
OBJ_DIR       := $(BIN_DIR)/obj

LIB_DIR       := ../lib

CFLAGS   := $(MCU) -Wall -Wextra -g -O0 -std=gnu11 -DSTM32G474xx \
            -I$(HDR_DIR) \
            -I$(LIB_DIR) \
            -I$(CMSIS_CORE)/CMSIS/Core/Include \
            -I$(CMSIS_DEVICE)/Include
DEPFLAGS := -MMD -MP

PROGRAMS := $(basename $(notdir $(wildcard $(SRC_DIR)/*.c)))

# Top-level project dispatch: lets you run `make <project>` /
# `make flash-<project>` from Embedded_Projects/ itself, without typing
# -C <project> -f ../Makefile every time. Only populated when invoked
# from the top level (PROJECTS is empty once you're -C'd into a
# project, so this never collides with the per-program rules below).
PROJECTS := $(sort $(foreach f,$(wildcard */src/*.c),$(firstword $(subst /, ,$(f)))))
MAKEFILE_PATH := $(abspath $(firstword $(MAKEFILE_LIST)))

define PROJECT_template
.PHONY: $(1) flash-$(1) clean-$(1)
$(1):
	$$(MAKE) -C $(1) -f $$(MAKEFILE_PATH) $(1)

flash-$(1):
	$$(MAKE) -C $(1) -f $$(MAKEFILE_PATH) flash-$(1)

clean-$(1):
	$$(MAKE) -C $(1) -f $$(MAKEFILE_PATH) clean
endef

$(foreach proj,$(PROJECTS),$(eval $(call PROJECT_template,$(proj))))

# Support `make clean <project> [<project> ...]` from the top level:
# extra words after "clean" are project names whose bin/ should be
# wiped, not build targets. This overrides any build rule those names
# would otherwise have (from PROJECT_template above) with a no-op, so
# `make clean blinking_led` cleans instead of building it.
ifeq (clean,$(firstword $(MAKECMDGOALS)))
CLEAN_TARGETS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
ifneq ($(CLEAN_TARGETS),)
$(eval $(CLEAN_TARGETS):;@:)
endif
endif

# `make project <name>`: scaffold a new project directory with a
# starter source file and open it. Extra word after "project" is
# the name, not a build target (same trick as `clean` above).
ifeq (project,$(firstword $(MAKECMDGOALS)))
PROJECT_NAME := $(word 2,$(MAKECMDGOALS))
ifneq ($(PROJECT_NAME),)
$(eval $(PROJECT_NAME):;@:)
endif
endif

.PHONY: project
project:
ifeq ($(PROJECT_NAME),)
	$(error Usage: make project <name>)
endif
	mkdir -p $(PROJECT_NAME)
	mkdir -p $(PROJECT_NAME)/src
	touch $(PROJECT_NAME)/src/$(PROJECT_NAME).c
	code $(PROJECT_NAME)/src/$(PROJECT_NAME).c

# Disable make's built-in implicit rules/variables -- same reasoning as
# on the host side: avoids silent fallback to the wrong compiler/flags.
MAKEFLAGS += -r -R

# `make lib <name>`: compile a single lib/*/<name>.c file without linking --
# a fast syntax/compile check for one driver file. <name> can be given as
# "lpuart_driver", "lpuart_driver.c", or "uart_driver/lpuart_driver.c".
# Works from the top level (Embedded_Projects/) or from inside a project
# dir. Extra word after "lib" is the file name, not a build target (same
# MAKECMDGOALS trick as `clean`/`project` above).
ifeq (lib,$(firstword $(MAKECMDGOALS)))
LIB_ARG := $(word 2,$(MAKECMDGOALS))
ifneq ($(LIB_ARG),)
$(eval $(LIB_ARG):;@:)
endif
endif

.PHONY: lib
ifneq ($(PROJECTS),)
# top level: no project dir means LIB_DIR/OBJ_DIR wouldn't resolve here --
# forward into any one project dir, since lib/ is a shared sibling of all
# of them and the relative path (../lib) is identical from any project.
lib:
ifeq ($(LIB_ARG),)
	$(error Usage: make lib <name>  e.g. make lib lpuart_driver)
endif
	@$(MAKE) -C $(firstword $(PROJECTS)) -f $(MAKEFILE_PATH) lib $(LIB_ARG)
else
# nested (inside a project dir): resolve the file and compile it via the
# existing $(OBJ_DIR)/lib/%.o pattern rule -- no duplicated compile logic.
LIB_REL := $(basename $(LIB_ARG))
ifeq ($(findstring /,$(LIB_REL)),)
LIB_REL := $(patsubst $(LIB_DIR)/%.c,%,$(firstword $(wildcard $(LIB_DIR)/*/$(LIB_REL).c)))
endif

lib: $(if $(LIB_REL),$(OBJ_DIR)/lib/$(LIB_REL).o)
ifeq ($(LIB_ARG),)
	$(error Usage: make lib <name>  e.g. make lib lpuart_driver)
endif
ifeq ($(LIB_REL),)
	$(error No source file found matching '$(LIB_ARG).c' under $(LIB_DIR)/*/)
endif
	@echo "compiled $(LIB_DIR)/$(LIB_REL).c -> $(OBJ_DIR)/lib/$(LIB_REL).o"
endif

.PHONY: all clean list
all: $(addprefix $(BIN_DIR)/,$(addsuffix .elf,$(PROGRAMS)))

COMMON_OBJS := $(OBJ_DIR)/startup_stm32g474xx.o $(OBJ_DIR)/system_stm32g4xx.o

# Shared driver code in ../lib/<driver>/*.c -- compiled and linked into
# every project. Unused code is stripped at link time by --gc-sections,
# so projects that don't call a driver don't pay for it in flash.
LIB_SRCS := $(wildcard $(LIB_DIR)/*/*.c)
LIB_OBJS := $(patsubst $(LIB_DIR)/%.c,$(OBJ_DIR)/lib/%.o,$(LIB_SRCS))

define PROGRAM_template
HDRS_$(1) := $$(shell $(CC) $(CFLAGS) -MM $(SRC_DIR)/$(1).c 2>/dev/null \
               | tr -d '\\' | tr ' ' '\n' \
               | grep -E '^$(HDR_DIR)/.*\.h$$$$')
OBJS_$(1) := $(OBJ_DIR)/$(1).o $$(patsubst $(HDR_DIR)/%.h,$(OBJ_DIR)/headers/%.o,$$(HDRS_$(1))) $(LIB_OBJS) $(COMMON_OBJS)

$(BIN_DIR)/$(1).elf: $$(OBJS_$(1)) $(LINKER_SCRIPT) | $(BIN_DIR)
	$(CC) $(MCU) -T$(LINKER_SCRIPT) -Wl,--gc-sections -Wl,-Map=$(BIN_DIR)/$(1).map -nostartfiles $$(filter-out $(LINKER_SCRIPT),$$^) -o $$@
	$(SIZE) $$@
	$(OBJCOPY) -O ihex $$@ $(BIN_DIR)/$(1).hex
	$(OBJCOPY) -O binary $$@ $(BIN_DIR)/$(1).bin

# make blink -> build only, no flash
$(1): $(BIN_DIR)/$(1).elf
	@:

# make flash-blink -> build + program via OpenOCD/ST-Link
flash-$(1): $(BIN_DIR)/$(1).elf
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg \
	  -c "reset_config srst_only srst_nogate connect_assert_srst" \
	  -c "program $$< verify reset exit"
endef

$(foreach prog,$(PROGRAMS),$(eval $(call PROGRAM_template,$(prog))))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/headers/%.o: $(HDR_DIR)/%.c | $(OBJ_DIR)/headers
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/lib/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/startup_stm32g474xx.o: $(STARTUP_DIR)/startup_stm32g474xx.s | $(OBJ_DIR)
	$(CC) $(MCU) -c $< -o $@

$(OBJ_DIR)/system_stm32g4xx.o: $(STARTUP_DIR)/system_stm32g4xx.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR) $(OBJ_DIR)/headers $(BIN_DIR):
	mkdir -p $@

-include $(wildcard $(OBJ_DIR)/*.d $(OBJ_DIR)/headers/*.d $(OBJ_DIR)/lib/*/*.d)

clean:
ifneq ($(CLEAN_TARGETS),)
	@for p in $(CLEAN_TARGETS); do rm -rf $$p/bin; echo "cleaned $$p/bin"; done
else
	rm -rf $(BIN_DIR)
endif

list:
	@for d in */; do \
	  d=$${d%/}; \
	  if ls $$d/src/*.c >/dev/null 2>&1; then \
	    echo "$$d:"; \
	    for f in $$d/src/*.c; do echo "  $$(basename "$$f" .c)"; done; \
	  fi; \
	done
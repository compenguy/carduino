PWD=$(shell pwd)
ifeq ("$(origin V)", "command line")
    VERBOSE=$(V)
endif
ifndef VERBOSE
    VERBOSE=0
endif

ifeq ($(VERBOSE),1)
    Q=
else
    Q=@
endif


BUILD_DIR=$(PWD)/bin
CLI_TOOL=arduino-cli

ARDUINO_CORE=arduino:mbed_nano
ARDUINO_BOARD=arduino:mbed_nano:nanorp2040connect
ARDUINO_PORT=/dev/ttyACM0
DEPS=ArduinoBLE \
     Arduino_LSM6DSOX
SKETCH=carduino.ino
SRCS=$(SKETCH)

BIN=$(SKETCH).bin

$(BUILD_DIR)/$(BIN): $(SRCS) sketch.json
	@echo "  COMPILE: $(SKETCH)"
	$(Q)$(CLI_TOOL) compile --build-path "$(BUILD_DIR)"

#sketch.json:
#	@echo "  CONFIG: attach to $(ARDUINO_BOARD)"
#	$(Q)$(CLI_TOOL) board attach $(ARDUINO_BOARD)
#	$(Q)$(CLI_TOOL) board attach serial://$(ARDUINO_PORT)

.PHONY: program build check-init config-init clean
program: build
	@echo "  PROGRAM: $(ARDUINO_BOARD)"
	$(Q)$(CLI_TOOL) upload

build: $(BUILD_DIR)/$(SKETCH).bin

check-init:
	@echo "  CONFIG: connected board details"
	$(Q)$(CLI_TOOL) board details $(ARDUINO_BOARD)
	@echo "  CONFIG: installed libraries"
	$(Q)$(CLI_TOOL) lib list

config-init:
	@echo "  CONFIG: arduino-cli update"
	$(Q)$(CLI_TOOL) update
	$(Q)$(CLI_TOOL) upgrade
	@echo "  CONFIG: arduino-cli install deps"
	$(Q)$(CLI_TOOL) core install $(ARDUINO_CORE)
	$(Q)$(CLI_TOOL) lib install $(DEPS)

clean:
	@echo "  CLEAN: $(BUILD_DIR)"
	$(Q)$(if $(BUILD_DIR),rm -fR "$(BUILD_DIR)")

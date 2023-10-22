# Thanks to Chase Lambert (https://makefiletutorial.com/#makefile-cookbook) and 
#  Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)

TARGET_EXEC := unpack
BUILD_DIR := ./build
SRC_DIRS := ./src
INSTALL_DIR := ${HOME}/bin

SRCS := $(shell find $(SRC_DIRS) -name '*.c')

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CC=gcc
CFLAGS=$(INC_FLAGS) -O3 -Wall -MMD -MP

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

all: $(BUILD_DIR)/$(TARGET_EXEC)

debug: CFLAGS += -g3 -O0 -DDEBUG=1
debug: clean all


install: all
	cp $(BUILD_DIR)/$(TARGET_EXEC) ${INSTALL_DIR}

-include $(DEPS)

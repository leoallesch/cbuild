# clang-format off

PLATFORM_DIR  := posix

# Root of the library (must be set by parent or environment)
LIB_HOME      ?= .

BUILD_DIR     := $(LIB_HOME)/build
ARTIFACTS_DIR := $(BUILD_DIR)/artifacts/bootstrap
BIN_DIR       := $(BUILD_DIR)/bin

CORE_LIB      := $(BIN_DIR)/libcore.a
CBUILD_LIB    := $(BIN_DIR)/libcbuild.a

CC            ?= gcc
AR            ?= ar

# Core library source directories
CORE_SRC_DIRS := \
	src/core/allocators \
	src/core/containers \
	src/core/logger \
	src/core/memory \
	src/core/os \
	src/core/string \
	src/core/platform/$(PLATFORM_DIR)/core/memory \
	src/core/platform/$(PLATFORM_DIR)/core/os

# CBuild library source directories
CBUILD_SRC_DIRS := \
	src/cbuild \
	src/cbuild/builder \
	src/cbuild/builder/steps \
	src/cbuild/cli \
	src/cbuild/graph

ALL_SRC_DIRS := $(CORE_SRC_DIRS) $(CBUILD_SRC_DIRS)

INC_DIRS := $(ALL_SRC_DIRS) include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS      += $(INC_FLAGS)
CFLAGS        += -Wall -Wextra -Werror -O0 -g -MMD

# Collect source files for each library
CORE_SRCS := $(foreach dir,$(CORE_SRC_DIRS),$(wildcard $(dir)/*.c))
CBUILD_SRCS := $(foreach dir,$(CBUILD_SRC_DIRS),$(wildcard $(dir)/*.c))

CORE_OBJS := $(CORE_SRCS:%=$(ARTIFACTS_DIR)/%.o)
CBUILD_OBJS := $(CBUILD_SRCS:%=$(ARTIFACTS_DIR)/%.o)

ALL_OBJS := $(CORE_OBJS) $(CBUILD_OBJS)
DEPS := $(ALL_OBJS:.o=.d)

CBUILD_EXE    := cbuild

# Default target: build both libraries and bootstrap executable
all: $(CBUILD_EXE)

$(CBUILD_EXE): $(CBUILD_LIB) build.c
	@echo "LINK $@"
	@$(CC) $(INC_FLAGS) build.c $(CBUILD_LIB) -o $@

# Build step for C source
$(ARTIFACTS_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	@echo "CC  $(notdir $<)"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Archive rules
$(CORE_LIB): $(CORE_OBJS)
	@mkdir -p $(dir $@)
	@echo "AR  $@"
	@$(AR) rcs $@ $^

$(CBUILD_LIB): $(CBUILD_OBJS) $(CORE_LIB)
	@mkdir -p $(dir $@)
	@echo "AR  $@"
	@$(AR) rcs $@ $(CBUILD_OBJS) $(CORE_OBJS)

# Cleanup
clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all clean

# Include dependency files
-include $(DEPS)

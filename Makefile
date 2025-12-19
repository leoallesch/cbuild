#// clang-format off

TARGET := cbuild

PLATFORM_DIR := posix

INC_DIRS := include

# SRC_FILES := build.c

SRC_DIRS := \
	src \
	src/cli \
	src/core \
	src/core/memory \
	src/core/allocators \
	src/core/containers \
	src/core/logger \
	src/core/os \
	src/core/string \
	src/graph \
	src/builder \
	src/platform \
	src/platform/$(PLATFORM_DIR)/core/memory \
	src/platform/$(PLATFORM_DIR)/core/os

include lib/makebuilder/builder.mk

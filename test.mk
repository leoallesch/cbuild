#// clang-format off

TARGET := test

BUILD_DIR := build

PLATFORM_DIR := posix

SRC_DIRS := \
	src/core \
	src/core/memory \
	src/core/allocators \
	src/core/containers \
	src/core/logger \
	src/core/os \
	src/core/string \
	src/graph \
	src/platform \
	src/platform/$(PLATFORM_DIR)/core/memory \
	src/platform/$(PLATFORM_DIR)/core/os \
	tests \
	tests/core/memory \
	tests/core/string \

include lib/makebuilder/lib_cpputest.mk
include lib/makebuilder/builder.mk

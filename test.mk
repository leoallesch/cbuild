TARGET := test

BUILD_DIR := build

SRC_DIRS := \
src/core \
	src/core/allocator \
	src/platform \
	src/platform/posix \
	src/platform/win32 \
	tests \
	tests/core/allocator \

INC_DIRS := include

include lib/makebuilder/lib_cpputest.mk
include lib/makebuilder/builder.mk


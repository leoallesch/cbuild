TARGET := cbuild

SRC_DIRS := \
	src/core \
	src/core/memory \
	src/platform \
	src/platform/posix \
	src/platform/win32

SRC_FILES := src/main.c

include lib/makebuilder/builder.mk

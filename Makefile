CC = gcc
CFLAGS = -fPIC -Wall -Wextra -Werror -Wfatal-errors -Wno-unused-parameter -finline-functions \
		 -fopenmp -pthread -Wno-unused-function -Iinclude -Wno-unused-result -Wno-maybe-uninitialized -Wno-unused-but-set-variable -Wno-unused-variable
CFLAGS_DEV = -g3 -D DEBUG_BUILD
CFLAGS_RELEASE = -O2 -D RELEASE_BUILD
ifeq ($(OS), Windows_NT)
	LINKER_FLAGS=-Llib glfw3.dll
	SHARED_EXT=dll
else
	LINKER_FLAGS=-lm lib/libglfw3.a
	SHARED_EXT=so
endif
NAME = st
DLL_NAME = soultaker

SOURCES  = $(shell find src lib -name "*.c")
PLUGIN_SOURCES = $(shell find plugins -name "*.c")
DEPSH    = $(shell find build -name "*.d" -exec grep -Eoh "[^ ]+.h" {} +)
OBJS_DEV = $(SOURCES:%.c=build/dev/%.o)
PLUGIN_OBJS_DEV = $(PLUGIN_SOURCES:%.c=build/dev/%.o)
OBJS_REL = $(SOURCES:%.c=build/release/%.o)
PLUGIN_OBJS_REL = $(PLUGIN_SOURCES:%.c=build/release/%.o)
DEPS_DEV = $(OBJS_DEV:%.o=%.d)
PLUGIN_DEPS_DEV = $(PLUGIN_OBJS_DEV:%.o=%.d)
DEPS_REL = $(OBJS_REL:%.o=%.d)
PLUGIN_DEPS_REL = $(PLUGIN_OBJS_REL:%.o=%.d)

all: dev

dev: build dev-folders dev-src dev-dll clean-data
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) main.c bin/dev/$(DLL_NAME).$(SHARED_EXT) -o bin/dev/$(NAME)

dev-folders:
	@mkdir -p bin/dev
	@mkdir -p bin/dev/plugins

dev-src: $(OBJS_DEV)
	@$(CC) -shared $(CFLAGS) $(CFLAGS_DEV) $(OBJS_DEV) $(LINKER_FLAGS) -o bin/dev/$(DLL_NAME).$(SHARED_EXT)

dev-dll: dev-src $(PLUGIN_OBJS_DEV)
	@$(CC) -shared $(LINKER_FLAGS) $(PLUGIN_OBJS_DEV) bin/dev/$(DLL_NAME).$(SHARED_EXT) -o bin/dev/plugins/$(DLL_NAME).$(SHARED_EXT)

build/dev/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@echo $<
	@$(CC) -MMD $(CFLAGS) $(CFLAGS_DEV) -fpic -c -o $@ $<

release: build release-folders release-src release-dll
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) main.c bin/release/$(DLL_NAME).$(SHARED_EXT) -o bin/release/$(NAME)

release-folders:
	@mkdir -p bin/release
	@mkdir -p bin/release/plugins

release-src: $(OBJS_REL)
	@$(CC) -shared $(CFLAGS) $(CFLAGS_RELEASE) $(OBJS_REL) $(LINKER_FLAGS) -o bin/release/$(DLL_NAME).$(SHARED_EXT)

release-dll: release-src $(PLUGIN_OBJS_REL)
	@$(CC) -shared $(CFLAGS) $(CFLAGS_RELEASE) $(PLUGIN_OBJS_REL) $(LINKER_FLAGS) bin/release/$(DLL_NAME).$(SHARED_EXT) -o bin/release/plugins/$(DLL_NAME).$(SHARED_EXT)

build/release/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@echo $<
	@$(CC) -MMD $(CFLAGS) $(CFLAGS_RELEASE) -c -o $@ $<

build:
	@mkdir -p build

-include $(DEPS_DEV)
-include $(DEPS_REL)
-include $(PLUGIN_DEPS_DEV)
-include $(PLUGIN_DEPS_REL)

clean-data:
	rm -f data/*
clean:
	rm -rf build bin

.PHONY: clean clean-data



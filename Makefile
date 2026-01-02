CC = gcc
CFLAGS = -MMD -Wall -Wextra -Werror -Wfatal-errors -Wno-unused-parameter -finline-functions \
		 -fopenmp -pthread -Wno-unused-function -Iinclude
CFLAGS_DEV = -g3 -D DEBUG_BUILD
CFLAGS_RELEASE = -O2 -D RELEASE_BUILD
LINKER_FLAGS = -Llib -lm -lglfw3dll
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

dev: build dev-src dev-dll clean-data

dev-src: $(OBJS_DEV)
	@mkdir -p bin/dev
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) $(OBJS_DEV) $(LINKER_FLAGS) -o bin/dev/$(NAME)

dev-dll: $(PLUGIN_OBJS_DEV)
	@mkdir -p bin/dev
	@mkdir -p bin/dev/plugins
	@$(CC) -shared $(CFLAGS) $(CFLAGS_DEV) $(PLUGIN_OBJS_DEV) $(LINKER_FLAGS) -o bin/dev/plugins/$(DLL_NAME).dll

build/dev/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@echo $<
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) $(LINKER_FLAGS) -c -o $@ $<

release: build release-src release-dll

release-src: $(OBJS_REL)
	@mkdir -p bin/release
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(OBJS_REL) $(LINKER_FLAGS) -o bin/release/$(NAME)

release-dll: $(PLUGIN_OBJS_REL)
	@mkdir -p bin/release
	@mkdir -p bin/release/plugins
	@$(CC) -shared $(CFLAGS) $(CFLAGS_RELEASE) $(PLUGIN_OBJS_REL) $(LINKER_FLAGS) -o bin/release/plugins/$(DLL_NAME).dll

build/release/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@echo $<
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LINKER_FLAGS) -c -o $@ $<

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



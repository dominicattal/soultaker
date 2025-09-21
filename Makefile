CC = gcc
CFLAGS = -MMD -Wall -Wextra -Werror -Wfatal-errors -Wno-cast-function-type -Wno-unused-parameter -Wno-stringop-overflow -Wno-use-after-free -fopenmp -pthread
CFLAGS_DEV = -g3 -D DEBUG_BUILD
CFLAGS_RELEASE = -O3 -D RELEASE_BUILD
LINKER_FLAGS = -lglfw3dll -lm -lOpenAL32 -lsndfile
DIR_LIB = lib
DIR_SRC = src
DIR_PLUG = plugins
DIR_OBJ = build
DIR_DEP = build
DIR_BIN = bin
DIR_DATA = data
NAME = prog
DLL_NAME = soultaker

$(shell mkdir -p build)
DIRS_LIB = $(shell find $(DIR_LIB) -type d -name "*link")
DIRS_INC = $(shell find $(DIR_LIB) -type d -name "*include")
HEADERS  = $(shell find $(DIR_SRC) $(DIR_LIB) -name "*.h")
SOURCES  = $(shell find $(DIR_SRC) $(DIR_LIB) -name "*.c")
PLUGIN_SOURCES = $(shell find $(DIR_PLUG) -name "*.c")
DEPSH    = $(shell find build -name "*.d" -exec grep -Eoh "[^ ]+.h" {} +)
INCLUDES = $(patsubst %, -I./%, $(DIRS_INC))
LIBS     = $(patsubst %, -L./%, $(DIRS_LIB))
OBJS_DEV = $(SOURCES:%.c=$(DIR_OBJ)/dev/%.o)
PLUGIN_OBJS_DEV = $(PLUGIN_SOURCES:%.c=$(DIR_OBJ)/dev/%.o)
OBJS_REL = $(SOURCES:%.c=$(DIR_OBJ)/release/%.o)
PLUGIN_OBJS_REL = $(PLUGIN_SOURCES:%.c=$(DIR_OBJ)/release/%.o)
DEPS_DEV = $(OBJS_DEV:%.o=%.d)
PLUGIN_DEPS_DEV = $(PLUGIN_OBJS_DEV:%.o=%.d)
DEPS_REL = $(OBJS_REL:%.o=%.d)
PLUGIN_DEPS_REL = $(PLUGIN_OBJS_REL:%.o=%.d)

all: dev

dev: dev-src dev-dll clean-data

dev-src: $(OBJS_DEV)
	@mkdir -p $(DIR_BIN)/dev
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) $(LIBS) $(INCLUDES) $(OBJS_DEV) $(LINKER_FLAGS) -o $(DIR_BIN)/dev/$(NAME)

dev-dll: $(PLUGIN_OBJS_DEV)
	@mkdir -p $(DIR_BIN)/dev
	@mkdir -p $(DIR_BIN)/dev/plugins
	@$(CC) -shared $(CLAFGS) $(CLFAGS_DEV) $(LIBS) $(INCLUDES) $(PLUGIN_OBJS_DEV) $(LINKER_FLAGS) -o $(DIR_BIN)/dev/plugins/$(DLL_NAME).dll

$(DIR_OBJ)/dev/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@echo $<
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) $(LIBS) $(INCLUDES) $(LINKER_FLAGS) -c -o $@ $<

release: release-src release-dll

release-src: $(OBJS_REL)
	@mkdir -p $(DIR_BIN)/release
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LIBS) $(INCLUDES) $(OBJS_REL) $(LINKER_FLAGS) -o $(DIR_BIN)/release/$(NAME)

release-dll: $(PLUGIN_OBJS_REL)
	@mkdir -p $(DIR_BIN)/release
	@mkdir -p $(DIR_BIN)/release/plugins
	@$(CC) -shared $(CFLAGS) $(CFLAGS_RELEASE) $(LIBS) $(INCLUDES) $(PLUGIN_OBJS_REL) $(LINKER_FLAGS) -o $(DIR_BIN)/release/plugins/$(DLL_NAME).dll

$(DIR_OBJ)/release/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@echo $<
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LINKER_FLAGS) $(LIBS) $(INCLUDES) -c -o $@ $<

-include $(DEPS_DEV)
-include $(DEPS_REL)
-include $(PLUGIN_DEPS_DEV)
-include $(PLUGIN_DEPS_REL)

clean-data:
	rm -f data/*
clean:
	rm -rf $(DIR_OBJ) $(DIR_DEP) $(DIR_BIN)
.PHONY: clean clean-data

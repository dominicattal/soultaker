CC = gcc
CFLAGS = -MMD -Wall -Wextra -Werror -Wfatal-errors -Wno-unused-parameter -Wno-stringop-overflow -fopenmp -pthread
CFLAGS_DEV = -g3 -D DEBUG_BUILD
CFLAGS_RELEASE = -O3 -D RELEASE_BUILD
LINKER_FLAGS = -lglfw3dll -lm -pthread -lOpenAL32 -lsndfile
DIR_LIB = lib
DIR_SRC = src
DIR_OBJ = build
DIR_DEP = build
DIR_BIN = bin
NAME = prog

$(shell mkdir -p build)
DIRS_LIB = $(shell find $(DIR_LIB) -type d -name "*link")
DIRS_INC = $(shell find $(DIR_LIB) -type d -name "*include")
HEADERS  = $(shell find $(DIR_SRC) $(DIR_LIB) -name "*.h")
SOURCES  = $(shell find $(DIR_SRC) $(DIR_LIB) -name "*.c")
DEPSH    = $(shell find build -name "*.d" -exec grep -Eoh "[^ ]+.h" {} +)
INCLUDES = $(patsubst %, -I./%, $(DIRS_INC))
LIBS     = $(patsubst %, -L./%, $(DIRS_LIB))
OBJS_DEV = $(SOURCES:%.c=$(DIR_OBJ)/dev/%.o)
OBJS_REL = $(SOURCES:%.c=$(DIR_OBJ)/release/%.o)
DEPS_DEV = $(OBJS_DEV:%.o=%.d)
DEPS_REL = $(OBJS_REL:%.o=%.d)

all: dev

dev: $(OBJS_DEV)
	@mkdir -p $(DIR_BIN)/dev
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) $(LIBS) $(INCLUDES) $(OBJS_DEV) $(LINKER_FLAGS) -o $(DIR_BIN)/dev/$(NAME)

$(DIR_OBJ)/dev/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) $(LIBS) $(INCLUDES) $(LINKER_FLAGS) -c -o $@ $<
	@echo $<

release: $(OBJS_REL)
	@mkdir -p $(DIR_BIN)/release
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LIBS) $(INCLUDES) $(OBJS_REL) $(LINKER_FLAGS) -o $(DIR_BIN)/release/$(NAME)

$(DIR_OBJ)/release/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LINKER_FLAGS) $(LIBS) $(INCLUDES) -c -o $@ $<
	@echo $<

-include $(DEPS_DEV)
-include $(DEPS_REL)

clean:
	rm -rf $(DIR_OBJ) $(DIR_DEP) $(DIR_BIN)
.PHONY: clean

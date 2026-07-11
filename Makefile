PROJECT_NAME = lambda-calculus
CC = gcc

ifeq ($(BUILD_MODE), DEBUG)
	ifeq ($(ASAN), 1)
	 	CFLAGS += -fsanitize=address -g -fno-omit-frame-pointer -O0
	else
		CFLAGS += -g -O0
	endif
else
	CFLAGS += -s -O1
endif

INCLUDE_PATHS = -Iinclude
SRC_DIR = src
OBJ_DIR = obj

SRC  = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,obj/%.o,$(SRC))

all: $(PROJECT_NAME)

$(OBJ_DIR):
	mkdir -p obj

$(PROJECT_NAME): $(OBJS)
	$(CC) -o $(PROJECT_NAME) $(OBJS) $(CFLAGS) $(INCLUDE_PATHS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS)

clean:
	rm -rf obj
	rm -f lambda-calculus

.PHONY: all clean
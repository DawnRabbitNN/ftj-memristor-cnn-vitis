CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS ?= -Isrc/core -Iconfig
LDLIBS ?= -lm

BUILD_DIR := build
TEST_BINARY := $(BUILD_DIR)/test_ftj_math
MAIN_SOURCE := legacy/main.c

.PHONY: all test check-main clean

all: test check-main

$(TEST_BINARY): src/core/ftj_math.c tests/test_ftj_math.c src/core/ftj_math.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) src/core/ftj_math.c tests/test_ftj_math.c -o $@ $(LDLIBS)

test: $(TEST_BINARY)
	./$(TEST_BINARY)

check-main: $(MAIN_SOURCE) src/core/ftj_math.h \
	config/board_config.example.h config/model_config.example.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -fsyntax-only $(MAIN_SOURCE)

clean:
	rm -rf $(BUILD_DIR)

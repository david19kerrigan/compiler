CC = gcc
CFLAGS = -g -I./src -I/usr/include
AC = nasm
AFLAGS = -felf64 -g
TEST_DIR = ./test
SRC_DIR = ./src
BUILD_DIR = ./build
MATH_TEST_FILES = math_1.c math_2.c math_3.c
ANS_MATH = 19 15 66
VARS_TEST_FILES = vars_1.c
ANS_VARS = 9

default: test

.PHONY: test
test:
	@i=1; \
	for test_file in $(VARS_TEST_FILES); do \
		expected=$$(echo $(ANS_VARS) | cut -d' ' -f$$((i))); \
		$(CC) $(CFLAGS) ./$(SRC_DIR)/compile.c -o $(BUILD_DIR)/compile.out ; \
		$(BUILD_DIR)/compile.out ../$(TEST_DIR)/"$$test_file" ; \
		cat $(BUILD_DIR)/main.asm ; \
		$(AC) $(AFLAGS) $(BUILD_DIR)/main.asm -o $(BUILD_DIR)/main.out ; \
		$(AC) $(AFLAGS) ./$(SRC_DIR)/lib.asm -o $(BUILD_DIR)/lib.out ; \
		ld $(BUILD_DIR)/main.out $(BUILD_DIR)/lib.out -o $(BUILD_DIR)/final.out ; \
		my_output=$$($(BUILD_DIR)/final.out) ; \
		if [ "$$my_output" != "$$expected" ]; then \
		    echo "Failure $$my_output != $$expected"; \
    	else \
		    echo "Success $$my_output == $$expected"; \
    	fi; \
    	i=$$((i+1)); \
	done

clean:
	rm $(BUILD_DIR)/*


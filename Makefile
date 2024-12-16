CC = gcc
CFLAGS = -g -I./src -I/usr/include
AC = nasm
AFLAGS = -felf64 -g
TEST_DIR = ./test
SRC_DIR = ./src
BUILD_DIR = ./build
TEST_FILES = math_1.c math_2.c math_3.c vars_1.c vars_2.c vars_3.c vars_4.c bool_1.c bool_2.c bool_3.c cond_1.c while_1.c while_2.c array_1.c
ANS = 19 15 66 9 21 42 1643628 1 0 1 12 4,3,2,1,0 1,2,3,4,5 0

default: test

.PHONY: test
test:
	@i=1; \
	for test_file in $(TEST_FILES); do \
		expected=$$(echo $(ANS) | cut -d' ' -f$$((i)) | sed 's/,/\n/g') ; \
		$(CC) $(CFLAGS) ./$(SRC_DIR)/compile.c -o $(BUILD_DIR)/compile.out ; \
		$(BUILD_DIR)/compile.out ../$(TEST_DIR)/"$$test_file" ; \
		#cat $(BUILD_DIR)/main.asm ; \
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
    	echo "--------------------------------------------------------------------------------------------------------------------------------------------------------------------------"; \
	done

asm:
	cat $(BUILD_DIR)/main.asm ; \
	$(AC) $(AFLAGS) $(BUILD_DIR)/main.asm -o $(BUILD_DIR)/main.out ; \
	$(AC) $(AFLAGS) ./$(SRC_DIR)/lib.asm -o $(BUILD_DIR)/lib.out ; \
	ld $(BUILD_DIR)/main.out $(BUILD_DIR)/lib.out -o $(BUILD_DIR)/final.out ; \
	$(BUILD_DIR)/final.out ; \

clean:
	rm $(BUILD_DIR)/*


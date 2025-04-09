CC = gcc -Werror
BUILD_DIR = Build
SRC_DIR = src
OUT_FILE = $(BUILD_DIR)/circular_buffer.out

OBJECT_FILES = $(BUILD_DIR)/main.o $(BUILD_DIR)/circular_buffer_sequential.o \
               $(BUILD_DIR)/circular_buffer_test.o

all: create_build_dir $(OBJECT_FILES)
	$(CC) $(OBJECT_FILES) -o $(OUT_FILE)

create_build_dir:
	@if [ ! -e $(BUILD_DIR) ]; then \
		mkdir $(BUILD_DIR); \
	fi

run: $(OUT_FILE)
	./$(OUT_FILE)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(BUILD_DIR)/circular_buffer_test.o
	$(CC) -c $(SRC_DIR)/main.c -o $@

$(BUILD_DIR)/circular_buffer_sequential.o: $(SRC_DIR)/circular_buffer_sequential.h \
                                           $(SRC_DIR)/circular_buffer_sequential.c
	$(CC) -c $(SRC_DIR)/circular_buffer_sequential.c -o $@

$(BUILD_DIR)/circular_buffer_test.o: $(SRC_DIR)/circular_buffer_test.h \
                                     $(SRC_DIR)/circular_buffer_test.c \
                                     $(BUILD_DIR)/circular_buffer_sequential.o
	$(CC) -c $(SRC_DIR)/circular_buffer_test.c -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY = create_build_dir $(OBJECT_FILES)

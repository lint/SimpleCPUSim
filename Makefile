TARGET := cpu_sim

BUILD_DIR := bld
SRC_DIR := src

# find source files
SOURCES=$(shell find $(SRC_DIR) -type f -name *.c)

# find header files
HEADERS=$(shell find $(SRC_DIR) -type f -name *.h)

# create object file paths
OBJECTS := $(SOURCES:%=$(BUILD_DIR)/%.o)

# build object files
$(BUILD_DIR)/%.c.o: %.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# build target executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@
	@echo "\nbuilt $(TARGET) in ./"
	@echo "run with './$(TARGET) <config_file> <input_file>'"

.PHONY: clean

# remove built objects and target executable
clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) $(TARGET)
TARGET := cpu_sim

BIN_DIR := bin
OBJ_DIR := obj
SRC_DIR := src

# find source files
SOURCES=$(shell find $(SRC_DIR) -type f -name *.c)

# find header files
HEADERS=$(shell find $(SRC_DIR) -type f -name *.h)

# create object file paths
OBJECTS := $(SOURCES:%=$(OBJ_DIR)/%.o)

# build object files
$(OBJ_DIR)/%.c.o: %.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# build target executable
$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJECTS) -o $@
	@echo "\nbuilt $(TARGET) in ./$(BIN_DIR)"
	@echo "run with './$(BIN_DIR)/$(TARGET) <input_file>' or './$(BIN_DIR)/$(TARGET) <config_file> <input_file>'"

# remove built objects and target executable
.PHONY: clean
clean:
	$(RM) -r $(OBJ_DIR)
	$(RM) $(BIN_DIR)/$(TARGET)
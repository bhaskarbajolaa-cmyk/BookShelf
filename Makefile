# --- Compiler Settings ---
CXX = g++

# Let pkg-config find the PostgreSQL headers automatically
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I./include $(shell pkg-config --cflags libpqxx)

# Let pkg-config handle the PostgreSQL linking automatically, plus add pthread for the web server
LDFLAGS = -L./lib $(shell pkg-config --libs libpqxx) -lpthread

# --- Directories ---
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Final executable name
TARGET = $(BIN_DIR)/bookshelf

# Find all .cpp files in the src/ directory
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Map .cpp files to .o (object) files in the obj/ directory
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# --- Build Rules ---

# Default rule when you type 'make'
all: directories $(TARGET)

# Rule to link everything together into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to compile each .cpp file into a .o object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Automatically create the obj/ and bin/ folders if they don't exist
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Rule to clean up old build files
clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BIN_DIR)/*

# Tell Make these aren't actual files
.PHONY: all directories clean
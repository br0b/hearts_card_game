# Compiler and flags
CXX = g++
CXXFLAGS = -I$(INCLUDE_DIR) -Wall -Wextra -Wpedantic -std=c++20 -g -pg

# Project directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Source files
SERVER_SRC = $(SRC_DIR)/kierki-serwer.cpp
CLIENT_SRC = $(SRC_DIR)/kierki-klient.cpp
COMMON_SRC = $(filter-out $(SERVER_SRC) $(CLIENT_SRC), $(wildcard $(SRC_DIR)/*.cpp))

# Header files
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

# Object files
SERVER_OBJ = $(BUILD_DIR)/kierki-serwer.o $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(COMMON_SRC))
CLIENT_OBJ = $(BUILD_DIR)/kierki-klient.o $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(COMMON_SRC))

# Targets
TARGET_SERVER = kierki-serwer
TARGET_CLIENT = kierki-klient

# Default target
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Server target
$(TARGET_SERVER): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Client target
$(TARGET_CLIENT): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Object files for server
$(BUILD_DIR)/kierki-serwer.o: $(SERVER_SRC) $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Object files for client
$(BUILD_DIR)/kierki-klient.o: $(CLIENT_SRC) $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Common object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET_SERVER) $(TARGET_CLIENT)

.PHONY: all clean


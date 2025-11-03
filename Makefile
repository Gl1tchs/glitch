# Define the target platform and build type
BUILD_TYPE ?= Debug
PLATFORM ?= $(shell uname -s)

# Set the build and cache directories
BUILD_DIR = build
OUT_DIR = build
CACHE_DIR = .glitch
TEST_EXEC = $(OUT_DIR)/tests/glitch-tests

# Available actions
.PHONY: build clean install test

all: build

# Configure CMake
configure_cmake:
	@echo "Configuring cmake..."
	cmake -S . -B $(BUILD_DIR) -DGL_BUILD_DYNAMIC_LIBS=0 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

# Build the engine
build: configure_cmake
	@echo "Building engine..."
	cmake --build $(BUILD_DIR)

# Run tests
test: build
	@if [ -f $(TEST_EXEC) ]; then \
		echo "Running test executable..."; \
		$(TEST_EXEC); \
	else \
		echo "Test application could not be found."; \
	fi

# Clean the build and cache directories
clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR) $(CACHE_DIR)

# Install the build
install:
	@echo "Installing..."
	cmake --install $(BUILD_DIR)

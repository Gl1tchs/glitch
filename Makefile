CMAKE=cmake
CMAKE_FLAGS=-GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug

BUILD_PATH=build/

all: configure build
	
configure:
	@echo "Configuring cmake..."
	$(CMAKE) -S . -B $(BUILD_PATH) $(CMAKE_FLAGS)
	
build:
	@echo "Building engine..."
	$(CMAKE) --build $(BUILD_PATH)

clean:
	@rm -rf $(BUILD_PATH) bin

.PHONY: all configure build clean

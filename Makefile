CMAKE_BUILD_DIR := build
EXECUTABLE := embedded_metrics_uds

.PHONY: all configure build clean

all: configure build

configure:
	@echo "🛠️ Configuring with CMake..."
	mkdir -p $(CMAKE_BUILD_DIR)
	cd $(CMAKE_BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Debug

build:
	@echo "🏗️ Building project..."
	cd $(CMAKE_BUILD_DIR) && cmake --build . -- -j4

clean:
	@echo "🧹 Cleaning..."
	rm -rf $(CMAKE_BUILD_DIR) $(EXECUTABLE)

.PHONY: docs compile watch-and-compile docs-server docs-install cleanup style-check style-fix
.DEFAULT_GOAL = compile-debug

# The build stamps itself (cmake/write_build_number.cmake) and the app shows that number, so the
# archive name stays the same from build to build and does not carry a branch or a date
LINUX_ZIP:=bin/linux/maszyna-reloaded-linux64.zip
WINDOWS_ZIP:=bin/windows/maszyna-reloaded-win64.zip
CMAKE_BUILD_JOBS=$(shell cores=$$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1); if [ "$$cores" -gt 2 ]; then echo $$((cores - 2)); else echo 1; fi)
CLANG_TIDY_BUILD_DIR=build-clang-tidy
CLANG_TIDY_COMPILE_COMMANDS_FILE=$(CLANG_TIDY_BUILD_DIR)/compile_commands.json
CLANG_TIDY_BINDINGS_FILE=$(CLANG_TIDY_BUILD_DIR)/godot-cpp/gen/include/godot_cpp/classes/node.hpp
LIBMASZYNA_DEBUG:=""
# The extension is built against double precision godot-cpp, so only a Godot built the same way
# can load it - a single precision binary dies with a glibc heap assertion while the module
# initialises. Override when your double precision build is named differently:
#   make release-linux GODOT=godot-double
GODOT?=godot-double
CMAKE_GODOTCPP_API_VERSION=4.7

#Helper for CLion so it would see generated bindings
generate-bindings:
	cmake -B cmake-build-debug
	cmake --build cmake-build-debug --target generate_bindings


docs:
	cd demo && $(GODOT) --doctool .. --gdextension-docs


cleanup:
	rm -rf bin
	rm -rf demo/bin
	rm -rf demo/addons/gut


cleanup-build-debug:
	rm -rf build-debug


cleanup-build-release:
	rm -rf build-release


cleanup-builds: cleanup-build-debug cleanup-build-release
	

compile-debug: $(CLANG_TIDY_COMPILE_COMMANDS_FILE) $(CLANG_TIDY_BINDINGS_FILE)
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DGODOTCPP_TARGET=template_debug -DLIBMASZYNA_DEBUG=$(LIBMASZYNA_DEBUG) -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION)
	cmake --build build-debug --parallel $(CMAKE_BUILD_JOBS)


compile-release:
	cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DGODOTCPP_TARGET=template_release -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION)
	cmake --build build-release --parallel $(CMAKE_BUILD_JOBS)


# Optimized, but still the template_debug library the editor loads - the one to profile on.
# compile-debug builds the vendored Mover at -O0, which makes the physics several times slower
# than it is in a shipped build and sends any frame-time investigation after the wrong subsystem.
# Overwrites the same .so as compile-debug; run that to go back.
compile-profiling:
	cmake -B build-profiling -DCMAKE_BUILD_TYPE=RelWithDebInfo -DGODOTCPP_TARGET=template_debug -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION)
	cmake --build build-profiling --parallel $(CMAKE_BUILD_JOBS)


compile-all: compile-debug compile-release


cross-compile-release: compile-release compile-windows-release


cross-compile-debug: $(CLANG_TIDY_COMPILE_COMMANDS_FILE) $(CLANG_TIDY_BINDINGS_FILE)
	cmake -B build-linux64-debug \
          -DCMAKE_BUILD_TYPE=Debug \
          -DGODOTCPP_TARGET="template_debug" \
          -DLIBMASZYNA_DEBUG=ON \
          -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION)
	cmake --build build-linux64-debug --parallel $(CMAKE_BUILD_JOBS)
	cmake -B build-win64-debug \
          -DCMAKE_BUILD_TYPE=Debug \
          -DGODOTCPP_TARGET="template_debug" \
          -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION) \
          -DGODOTCPP_PLATFORM=windows \
          -DCMAKE_SYSTEM_NAME=Windows \
          -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
          -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
          -DCMAKE_SIZEOF_VOID_P=8
	cmake --build build-win64-debug --parallel $(CMAKE_BUILD_JOBS)


compile-windows-debug:
	cmake -B build-win64-debug \
          -DCMAKE_BUILD_TYPE=Debug \
          -DGODOTCPP_TARGET="template_debug" \
          -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION) \
          -DGODOTCPP_PLATFORM=windows \
          -DCMAKE_SYSTEM_NAME=Windows \
          -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
          -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
          -DCMAKE_SIZEOF_VOID_P=8
	cmake --build build-win64-debug --parallel $(CMAKE_BUILD_JOBS)


compile-windows-release:
	cmake -B build-win64-release \
          -DCMAKE_BUILD_TYPE=Release \
          -DGODOTCPP_TARGET="template_release" \
          -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION) \
          -DGODOTCPP_PLATFORM=windows \
          -DCMAKE_SYSTEM_NAME=Windows \
          -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
          -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
          -DCMAKE_SIZEOF_VOID_P=8
	cmake --build build-win64-release --parallel $(CMAKE_BUILD_JOBS)


release-linux: compile-release
	mkdir -p bin/linux
	cd demo && $(GODOT) --headless --export-release "linux_x86_64" ../bin/linux/reloaded.zip
	mv bin/linux/reloaded.zip $(LINUX_ZIP)
	@echo "Exported: $(LINUX_ZIP)"


release-windows: compile-windows-release
	mkdir -p bin/windows
	cd demo && $(GODOT) --headless --export-release "windows_x86_64" ../bin/windows/reloaded.zip
	mv bin/windows/reloaded.zip $(WINDOWS_ZIP)
	@echo "Exported: $(WINDOWS_ZIP)"


release: release-linux release-windows


docs-install:
	cd docs && make install


docs-server:
	cd docs && make runserver


watch-and-compile:
	sh scripts/autocompile.sh


$(CLANG_TIDY_COMPILE_COMMANDS_FILE):
	@echo "Style: configuring clang-tidy database..." && \
	cmake --log-level=ERROR -S . -B $(CLANG_TIDY_BUILD_DIR) -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DLIBMASZYNA_STYLE_CHECK=ON -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION)

$(CLANG_TIDY_BINDINGS_FILE): $(CLANG_TIDY_BUILD_DIR)/CMakeCache.txt
	@echo "Style: generating clang-tidy bindings..." && \
	cmake --build $(CLANG_TIDY_BUILD_DIR) --target generate_bindings

style-check: $(CLANG_TIDY_COMPILE_COMMANDS_FILE) $(CLANG_TIDY_BINDINGS_FILE)
	@scripts/style-check $(STYLE_FILE)


style-fix: $(CLANG_TIDY_COMPILE_COMMANDS_FILE) $(CLANG_TIDY_BINDINGS_FILE)
	@scripts/style-fix $(STYLE_FILE)

docker-build-tests:
	docker build -t godot-tests .


docker-run-tests: docker-build-tests
	docker run --rm godot-tests


run-tests: compile
	godot --path demo --headless -s addons/gut/gut_cmdln.gd -gdir=res://tests/ -gexit

.PHONY: linux-sdk-image compile-release-linux docs compile watch-and-compile docs-server docs-install cleanup style-check style-fix compile-release-symbols release-linux-symbols
.DEFAULT_GOAL = compile-debug

# The app shows the build number (cmake/write_build_number.cmake), so the archive name stays the
# same from build to build and does not carry a branch or a date
LINUX_ZIP:=bin/linux/maszyna-reloaded-linux64.zip
WINDOWS_ZIP:=bin/windows/maszyna-reloaded-win64.zip
BUILD_NUMBER_FILE:=demo/build_number.txt
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
# The engine the release is exported with - it has to match the editor exactly, since the export
# looks its template up by this version
GODOT_VERSION:=4.7.2

# glibc is only forward compatible: a library or template linked against a rolling distribution's
# glibc (2.43-2.44 here) refuses to start on anything older - Ubuntu 22.04/24.04, Debian 12, Mint.
# The Linux release is therefore built in ci/docker/linux-sdk, Godot's own buildroot SDK
# (glibc 2.34). The checkout is mounted at its own path and the build runs as the host user, so
# the cmake cache and every output land exactly where a host build would put them.
LINUX_SDK_IMAGE:=maszyna-linux-sdk
LINUX_SDK_RUN=docker run --rm --user $(shell id -u):$(shell id -g) -v $(CURDIR):$(CURDIR) -w $(CURDIR) $(LINUX_SDK_IMAGE)
GODOT_SOURCE_DIR:=build-godot-$(GODOT_VERSION)
LINUX_TEMPLATE:=$(GODOT_SOURCE_DIR)/bin/godot.linuxbsd.template_release.double.x86_64
LINUX_TEMPLATE_INSTALLED:=$(HOME)/.local/share/godot/export_templates/$(GODOT_VERSION).stable.double/linux_release.x86_64

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
	

# The build number is stamped only when it is asked for, so a build never changes it. That is
# what lets `upgrade-linux.sh` and `upgrade-windows.sh` - two separate make invocations, often
# hours apart - ship the same number, and it makes the number describe a release rather than the
# last time somebody compiled anything. Bump it deliberately: `make build-number`.
.PHONY: build-number
build-number:
	cmake -DOUT=$(BUILD_NUMBER_FILE) -P cmake/write_build_number.cmake

# A checkout that has never been stamped gets a number on its first build, and keeps it.
$(BUILD_NUMBER_FILE):
	$(MAKE) build-number


compile-debug: $(BUILD_NUMBER_FILE) $(CLANG_TIDY_COMPILE_COMMANDS_FILE) $(CLANG_TIDY_BINDINGS_FILE)
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DGODOTCPP_TARGET=template_debug -DLIBMASZYNA_DEBUG=$(LIBMASZYNA_DEBUG) -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION)
	cmake --build build-debug --parallel $(CMAKE_BUILD_JOBS)


compile-release: $(BUILD_NUMBER_FILE)
	cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DGODOTCPP_TARGET=template_release -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION)
	cmake --build build-release --parallel $(CMAKE_BUILD_JOBS)


# Optimized, but still the template_debug library the editor loads - the one to profile on.
# compile-debug builds the vendored Mover at -O0, which makes the physics several times slower
# than it is in a shipped build and sends any frame-time investigation after the wrong subsystem.
# Overwrites the same .so as compile-debug; run that to go back.
compile-profiling: $(BUILD_NUMBER_FILE)
	cmake -B build-profiling -DCMAKE_BUILD_TYPE=RelWithDebInfo -DGODOTCPP_TARGET=template_debug -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION)
	cmake --build build-profiling --parallel $(CMAKE_BUILD_JOBS)


# A shipped build that keeps its symbols, for turning a core dump into names. The switch that
# matters is the build type, not a flag of ours: godot-cpp links with -s unless DEBUG_SYMBOLS is
# on (its cmake/common_compiler_flags.cmake), and DEBUG_SYMBOLS is Debug or RelWithDebInfo. The
# price is -O2 instead of -O3, so this build is for diagnosing a crash and not for measuring frame
# times. It writes the same library as compile-release, so rebuild that one afterwards.
compile-release-symbols: $(BUILD_NUMBER_FILE)
	cmake -B build-release-symbols -DCMAKE_BUILD_TYPE=RelWithDebInfo -DGODOTCPP_TARGET=template_release -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION)
	cmake --build build-release-symbols --parallel $(CMAKE_BUILD_JOBS)


release-linux-symbols: compile-release-symbols
	mkdir -p bin/linux
	cd demo && $(GODOT) --headless --export-release "linux_x86_64" ../bin/linux/reloaded.zip
	mv bin/linux/reloaded.zip $(LINUX_ZIP)
	@echo "Exported with symbols: $(LINUX_ZIP)"


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


compile-windows-debug: $(BUILD_NUMBER_FILE)
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


compile-windows-release: $(BUILD_NUMBER_FILE)
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


linux-sdk-image:
	docker build -q -t $(LINUX_SDK_IMAGE) ci/docker/linux-sdk


compile-release-linux: $(BUILD_NUMBER_FILE) linux-sdk-image
	$(LINUX_SDK_RUN) sh -c 'cmake -B build-release-linux -DCMAKE_BUILD_TYPE=Release -DGODOTCPP_TARGET=template_release -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION) && cmake --build build-release-linux --parallel $(CMAKE_BUILD_JOBS)'


$(GODOT_SOURCE_DIR):
	git clone --depth 1 --branch $(GODOT_VERSION)-stable https://github.com/godotengine/godot.git $@


# Built once per engine version - the editor's own template is linked against the host's glibc
$(LINUX_TEMPLATE): | $(GODOT_SOURCE_DIR) linux-sdk-image
	$(LINUX_SDK_RUN) sh -c 'cd $(GODOT_SOURCE_DIR) && scons platform=linuxbsd arch=x86_64 target=template_release precision=double production=yes -j$(CMAKE_BUILD_JOBS)'


$(LINUX_TEMPLATE_INSTALLED): $(LINUX_TEMPLATE)
	install -D $< $@


release-linux: compile-release-linux $(LINUX_TEMPLATE_INSTALLED)
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
	cmake --log-level=ERROR -S . -B $(CLANG_TIDY_BUILD_DIR) -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION)

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

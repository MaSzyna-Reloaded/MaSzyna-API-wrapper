.PHONY: docs compile watch-and-compile docs-server docs-install cleanup style-check style-fix
.DEFAULT_GOAL = compile-debug

GITREV=$(shell git rev-parse --abbrev-ref HEAD | sed -e 's/[^A-Za-z0-9]//g')
DATE=$(shell date +"%Y%m%d")
CMAKE_BUILD_JOBS=$(shell cores=$$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1); if [ "$$cores" -gt 2 ]; then echo $$((cores - 2)); else echo 1; fi)
CLANG_TIDY_BUILD_DIR=build-clang-tidy
LIBMASZYNA_DEBUG:=""

#Helper for CLion so it would see generated bindings
generate-bindings:
	cmake -B cmake-build-debug
	cmake --build cmake-build-debug --target generate_bindings

docs:
	cd demo && godot --doctool .. --gdextension-docs


cleanup:
	rm -rf bin
	rm -rf demo/bin
	rm -rf demo/addons/gut


cleanup-build-debug:
	rm -rf build-debug


cleanup-build-release:
	rm -rf build-release


cleanup-builds: cleanup-build-debug cleanup-build-release
	

compile-debug:
	cmake -B build-debug -DGODOTCPP_TARGET=template_debug -DLIBMASZYNA_DEBUG=$(LIBMASZYNA_DEBUG)
	cmake --build build-debug --parallel $(CMAKE_BUILD_JOBS)


compile-release:
	cmake -B build-release -DGODOTCPP_TARGET=template_release
	cmake --build build-release --parallel $(CMAKE_BUILD_JOBS)


compile-all: compile-debug compile-release


cross-compile-release:
	cmake -B build-linux64 \
          -DGODOTCPP_TARGET="template_release"
	cmake --build build-linux64 --parallel $(CMAKE_BUILD_JOBS)
	cmake -B build-win64 \
          -DGODOTCPP_TARGET="template_release" \
          -DGODOTCPP_PLATFORM=windows \
          -DCMAKE_SYSTEM_NAME=Windows \
          -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
          -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
          -DCMAKE_SIZEOF_VOID_P=8
	cmake --build build-win64 --parallel $(CMAKE_BUILD_JOBS)


cross-compile-debug:
	cmake -B build-linux64 \
          -DGODOTCPP_TARGET="template_debug" -DLIBMASZYNA_DEBUG=ON
	cmake --build build-linux64 --parallel $(CMAKE_BUILD_JOBS)
	cmake -B build-win64 \
          -DGODOTCPP_TARGET="template_debug" \
          -DGODOTCPP_PLATFORM=windows \
          -DCMAKE_SYSTEM_NAME=Windows \
          -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
          -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
          -DCMAKE_SIZEOF_VOID_P=8
	cmake --build build-win64 --parallel $(CMAKE_BUILD_JOBS)


release-linux:
	cmake -B build-linux64 \
          -DGODOTCPP_TARGET="template_release"
	cmake --build build-linux64 --parallel $(CMAKE_BUILD_JOBS)
	cd demo && godot --headless --export-release "linux_x86_64" ../bin/linux/reloaded.zip && mv ../bin/linux/reloaded.zip ../bin/linux/reloaded-$(GITREV)-$(DATE)-linux.zip


release-windows:
	cmake -B build-win64 \
          -DGODOTCPP_TARGET="template_release" \
          -DGODOTCPP_PLATFORM=windows \
          -DCMAKE_SYSTEM_NAME=Windows \
          -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
          -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
          -DCMAKE_SIZEOF_VOID_P=8
	cmake --build build-win64 --parallel $(CMAKE_BUILD_JOBS)
	cd demo && godot --headless --export-release "windows_x86_64" ../bin/windows/reloaded.zip && mv ../bin/windows/reloaded.zip ../bin/windows/reloaded-$(GITREV)-$(DATE)-windows.zip


release: release-linux release-windows


docs-install:
	cd docs && make install


docs-server:
	cd docs && make runserver


watch-and-compile:
	sh scripts/autocompile.sh


$(CLANG_TIDY_BUILD_DIR)/compile_commands.json:
	@echo "Style: configuring clang-tidy database..." && \
	cmake --log-level=ERROR -S . -B $(CLANG_TIDY_BUILD_DIR) -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON


style-check: $(CLANG_TIDY_BUILD_DIR)/compile_commands.json
	@scripts/style-check $(STYLE_FILE)


style-fix: $(CLANG_TIDY_BUILD_DIR)/compile_commands.json
	@scripts/style-fix $(STYLE_FILE)

docker-build-tests:
	docker build -t godot-tests .


docker-run-tests: docker-build-tests
	docker run --rm godot-tests


run-tests: compile
	godot --path demo --headless -s addons/gut/gut_cmdln.gd -gdir=res://tests/ -gexit

.PHONY: docs compile watch-and-compile docs-server docs-install cleanup run-clang-tidy run-clang-tidy-fix
.DEFAULT_GOAL = compile-debug

GITREV=$(shell git rev-parse --abbrev-ref HEAD | sed -e 's/[^A-Za-z0-9]//g')
DATE=$(shell date +"%Y%m%d")

docs:
	cd demo && godot --doctool .. --gdextension-docs


cleanup:
	rm -rf bin
	rm -rf demo/bin
	rm -rf demo/addons/gut


compile-debug:
	cmake -B build-debug -DGODOTCPP_TARGET=template_debug
	cmake --build build-debug


compile-release:
	cmake -B build-release -DGODOTCPP_TARGET=template_release
	cmake --build build-release


compile-all: compile-debug compile-release


cross-compile-release:
	cmake -B build-linux64 \
          -DGODOTCPP_TARGET="template_release"
	cmake --build build-linux64
	cmake -B build-win64 \
          -DGODOTCPP_TARGET="template_release" \
          -DGODOTCPP_PLATFORM=windows \
          -DCMAKE_SYSTEM_NAME=Windows \
          -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
          -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
          -DCMAKE_SIZEOF_VOID_P=8
	cmake --build build-win64


cross-compile-debug:
	cmake -B build-linux64 \
          -DGODOTCPP_TARGET="template_debug"
	cmake --build build-linux64
	cmake -B build-win64 \
          -DGODOTCPP_TARGET="template_debug" \
          -DGODOTCPP_PLATFORM=windows \
          -DCMAKE_SYSTEM_NAME=Windows \
          -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
          -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
          -DCMAKE_SIZEOF_VOID_P=8
	cmake --build build-win64


release-linux:
	cmake -B build-linux64 \
          -DGODOTCPP_TARGET="template_release"
	cmake --build build-linux64
	cd demo && godot --headless --export-release "linux_x86_64" ../bin/linux/reloaded.zip && mv ../bin/linux/reloaded.zip ../bin/linux/reloaded-$(GITREV)-$(DATE)-linux.zip


release-windows:
	cmake -B build-win64 \
          -DGODOTCPP_TARGET="template_release" \
          -DGODOTCPP_PLATFORM=windows \
          -DCMAKE_SYSTEM_NAME=Windows \
          -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
          -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
          -DCMAKE_SIZEOF_VOID_P=8
	cmake --build build-win64
	cd demo && godot --headless --export-release "windows_x86_64" ../bin/windows/reloaded.zip && mv ../bin/windows/reloaded.zip ../bin/windows/reloaded-$(GITREV)-$(DATE)-windows.zip


release: release-linux release-windows


docs-install:
	cd docs && make install


docs-server:
	cd docs && make runserver


watch-and-compile:
	sh scripts/autocompile.sh


run-clang-tidy:
	scons compiledb && \
		find src/ \( -name "*.cpp" -or -name "*.hpp" \) \
			-not -path "src/maszyna/*"\
			-not -path "src/gen/*"\
			-exec clang-tidy --fix-notes --quiet -p . {} +


run-clang-tidy-fix:
	scons compiledb && \
		find src/ \( -name "*.cpp" -or -name "*.hpp" \) \
			-not -path "src/maszyna/*"\
			-not -path "src/gen/*"\
			-exec clang-tidy --fix --fix-errors --quiet -p . {} +

docker-build-tests:
	docker build -t godot-tests .


docker-run-tests: docker-build-tests
	docker run --rm godot-tests


run-tests: compile
	godot --path demo --headless -s addons/gut/gut_cmdln.gd -gdir=res://tests/ -gexit
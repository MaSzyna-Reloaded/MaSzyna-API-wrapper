.PHONY: linux-sdk-image compile-release-linux compile-android-release compile-android-debug release-android godot-version docs compile watch-and-compile api-docs docs-server docs-install docs-pdf cleanup style-check style-fix compile-release-symbols release-linux-symbols
.DEFAULT_GOAL = compile-debug

# The app shows the build number (cmake/write_build_number.cmake), so the archive name stays the
# same from build to build and does not carry a branch or a date
LINUX_ZIP:=bin/linux/maszyna-reloaded-linux64.zip
ANDROID_APK:=bin/android/maszyna-reloaded-android-arm64.apk
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
GODOT_BIN:=$(GODOT_SOURCE_DIR)/bin
GODOT_SCONS=scons precision=double production=yes -j$(CMAKE_BUILD_JOBS)
LINUX_TEMPLATE:=$(GODOT_BIN)/godot.linuxbsd.template_release.double.x86_64
ANDROID_TEMPLATES:=$(GODOT_BIN)/android_release.apk $(GODOT_BIN)/android_debug.apk
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


# Any NDK will do for a GDExtension; GitHub's runners carry one in ANDROID_NDK_LATEST_HOME
ANDROID_NDK_ROOT?=$(ANDROID_NDK_LATEST_HOME)

compile-android-debug: $(BUILD_NUMBER_FILE)
	cmake -B build-android-debug -DCMAKE_BUILD_TYPE=Debug -DGODOTCPP_TARGET=template_debug -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION) -DGODOTCPP_PLATFORM=android -DANDROID_NDK_ROOT=$(ANDROID_NDK_ROOT)
	cmake --build build-android-debug --parallel $(CMAKE_BUILD_JOBS)


compile-android-release: $(BUILD_NUMBER_FILE)
	cmake -B build-android-release -DCMAKE_BUILD_TYPE=Release -DGODOTCPP_TARGET=template_release -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION) -DGODOTCPP_PLATFORM=android -DANDROID_NDK_ROOT=$(ANDROID_NDK_ROOT)
	cmake --build build-android-release --parallel $(CMAKE_BUILD_JOBS)


linux-sdk-image:
	docker build -q -t $(LINUX_SDK_IMAGE) ci/docker/linux-sdk


# The SDK container has no Godot, so the API the build binds against is dumped on the host first
extension_api.json:
	$(GODOT) --headless --dump-extension-api


compile-release-linux: $(BUILD_NUMBER_FILE) extension_api.json linux-sdk-image
	$(LINUX_SDK_RUN) sh -c 'cmake -B build-release-linux -DCMAKE_BUILD_TYPE=Release -DGODOTCPP_TARGET=template_release -DGODOTCPP_API_VERSION=$(CMAKE_GODOTCPP_API_VERSION) && cmake --build build-release-linux --parallel $(CMAKE_BUILD_JOBS)'


$(GODOT_SOURCE_DIR):
	git clone --depth 1 --branch $(GODOT_VERSION)-stable https://github.com/godotengine/godot.git $@


# Godot publishes no double precision build, so the engine is built here, once per engine version.
# CI does it in .github/workflows/godot-engine.yml and every other run fetches the result into
# $(GODOT_BIN) with ci/fetch-godot.sh, which is what keeps these rules from firing there.
godot-version:
	@echo $(GODOT_VERSION)


# The editor too is built in the SDK - the host's own is linked against the host's glibc, and this
# one runs on the CI runner as well
$(GODOT_BIN)/godot.linuxbsd.%.double.x86_64: | $(GODOT_SOURCE_DIR) linux-sdk-image
	$(LINUX_SDK_RUN) sh -c 'cd $(GODOT_SOURCE_DIR) && $(GODOT_SCONS) platform=linuxbsd arch=x86_64 target=$*'


# Direct3D 12 is not supported by the project, so the template is built without it and needs none
# of its SDK
$(GODOT_BIN)/godot.windows.%.double.x86_64.exe: | $(GODOT_SOURCE_DIR)
	cd $(GODOT_SOURCE_DIR) && $(GODOT_SCONS) platform=windows arch=x86_64 target=$* d3d12=no


# The export presets do not use a gradle build, so the export takes the ready APKs; scons puts the
# native libraries where gradle packs them from. Needs ANDROID_HOME and a JDK 17. Built without
# the Swappy frame pacing library, which scons refuses to build without otherwise.
$(ANDROID_TEMPLATES) &: | $(GODOT_SOURCE_DIR)
	cd $(GODOT_SOURCE_DIR) && $(GODOT_SCONS) platform=android arch=arm64 target=template_release swappy=no \
	    && $(GODOT_SCONS) platform=android arch=arm64 target=template_debug swappy=no \
	    && cd platform/android/java && ./gradlew generateGodotTemplates


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


release-android: compile-android-release
	mkdir -p bin/android
	cd demo && $(GODOT) --headless --export-release "android_arm64" ../$(ANDROID_APK)
	@echo "Exported: $(ANDROID_APK)"


release: release-linux release-windows


# The class reference pages of the Jekyll site in docs/ (docs/api/, not versioned): C++ classes
# from doc_classes/*.xml - refresh those with `make docs` - and GDScript classes from their ##
# comments
api-docs:
	scripts/make-api-docs docs/api


docs-install:
	cd docs && make install


# Generates the class reference and serves the site; without Ruby on the host, both run in Docker
docs-server:
	cd docs && make runserver


# The site as one PDF: the home page, the guides in the order of its menu and the class reference
# (scripts/make-docs-book). The diagrams are drawn by mermaid-cli and the book typeset by pandoc
# with XeLaTeX, both in Docker, as the host user so that bin/docs stays the host's
DOCS_PDF_DIR:=bin/docs
DOCS_BOOK_DIR:=$(DOCS_PDF_DIR)/book
DOCS_PDF:=$(DOCS_PDF_DIR)/maszyna-reloaded-core.pdf
DOCS_PANDOC_IMAGE:=pandoc/extra:3.7
DOCS_MERMAID_IMAGE:=minlag/mermaid-cli:11.4.2
DOCS_DOCKER_RUN:=docker run --rm --user "$(shell id -u):$(shell id -g)" -v "$(CURDIR):/data" -w /data
docs-pdf:
	rm -rf $(DOCS_BOOK_DIR)
	mkdir -p $(DOCS_BOOK_DIR)
	scripts/make-api-docs $(DOCS_BOOK_DIR)/api
	scripts/make-docs-book $(DOCS_BOOK_DIR)
	for diagram in $(DOCS_BOOK_DIR)/*.mmd; do \
		$(DOCS_DOCKER_RUN) $(DOCS_MERMAID_IMAGE) -q -b white -s 2 -i "/data/$$diagram" -o "/data/$${diagram%.mmd}.png" || exit 1; \
	done
	$(DOCS_DOCKER_RUN) $(DOCS_PANDOC_IMAGE) $(DOCS_BOOK_DIR)/book.md --metadata-file=scripts/docs-pdf.yaml \
		--resource-path=$(DOCS_BOOK_DIR):docs --pdf-engine=xelatex --toc --toc-depth=2 \
		--top-level-division=chapter -o $(DOCS_PDF)
	@echo "Documentation: $(DOCS_PDF)"


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

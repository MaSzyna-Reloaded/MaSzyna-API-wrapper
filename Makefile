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
	scons target=template_debug


compile-release:
	scons target=template_release maszyna_debug=off


compile-all: compile-debug compile-release


cross-compile-release:
	scons target=template_release platform=linux maszyna_debug=off
	scons target=template_release platform=windows maszyna_debug=off


cross-compile-debug:
	scons target=template_debug platform=linux
	scons target=template_debug platform=windows


release-linux:
	scons target=template_release platform=linux maszyna_debug=off
	cd demo && godot --headless --export-release "Linux" ../bin/linux/reloaded.zip && mv ../bin/linux/reloaded.zip ../bin/linux/reloaded-$(GITREV)-$(DATE)-linux.zip


release-windows:
	scons target=template_release platform=windows maszyna_debug=off
	cd demo && godot --headless --export-release "Windows Desktop" ../bin/windows/reloaded.zip && mv ../bin/windows/reloaded.zip ../bin/windows/reloaded-$(GITREV)-$(DATE)-windows.zip


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

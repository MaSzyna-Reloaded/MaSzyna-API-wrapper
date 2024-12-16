.PHONY: docs compile watch-and-compile docs-server docs-install cleanup
.DEFAULT_GOAL = compile

docs:
	cd demo && godot --doctool .. --gdextension-docs


cleanup:
	rm -rf bin
	rm -rf demo/bin


compile-debug:
	scons target=template_debug

compile-release:
	scons target=template_release

compile-all: compile-debug compile-release

cross-compile-release:
	scons target=template_release platform=linux
	scons target=template_release platform=windows

docs-install:
	cd docs && make install


docs-server:
	cd docs && make runserver


watch-and-compile:
	sh scripts/autocompile.sh

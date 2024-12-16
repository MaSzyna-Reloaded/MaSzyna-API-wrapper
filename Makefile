.PHONY: docs compile watch-and-compile docs-server docs-install cleanup
.DEFAULT_GOAL = compile

docs:
	cd demo && godot --doctool .. --gdextension-docs


cleanup:
	rm -rf bin
	rm -rf demo/bin


compile:
	scons


docs-install:
	cd docs && make install


docs-server:
	cd docs && make runserver


watch-and-compile:
	sh scripts/autocompile.sh

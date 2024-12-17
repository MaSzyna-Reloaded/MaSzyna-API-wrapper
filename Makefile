.PHONY: docs compile watch-and-compile docs-server docs-install cleanup
.DEFAULT_GOAL = compile

docs:
	cd demo && godot --doctool .. --gdextension-docs


cleanup:
	rm -rf bin
	rm -rf demo/bin
	rm -rf demo/addons/gut


compile:
	scons


docs-install:
	cd docs && make install


docs-server:
	cd docs && make runserver


watch-and-compile:
	sh scripts/autocompile.sh


docker-build-tests:
	docker build -t godot-tests .


docker-run-tests: docker-build-tests
	docker run --rm godot-tests


run-tests: compile
	godot --path demo --headless -s addons/gut/gut_cmdln.gd -gdir=res://tests/ -gexit

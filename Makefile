.PHONY=docs compile
.DEFAULT_GOAL =compile

docs:
	cd demo && godot --doctool .. --gdextension-docs

compile:
	scons

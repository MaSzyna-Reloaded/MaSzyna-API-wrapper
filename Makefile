.PHONY=docs compile watch-and-compile
.DEFAULT_GOAL =compile

docs:
	cd demo && godot --doctool .. --gdextension-docs


compile:
	scons

watch-and-compile:
	sh scripts/autocompile.sh

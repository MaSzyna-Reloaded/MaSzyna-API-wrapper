.PHONY: docs compile watch-and-compile docs-server docs-install cleanup run-clang-tidy run-clang-tidy-fix
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

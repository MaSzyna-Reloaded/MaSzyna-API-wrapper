# MaSzyna-API-wrapper

## About the project

This C++ GDExtension for Godot Engine offers a simplified interface to the MaSzyna train simulator API. Tailored for the MaSzyna Reloaded project, it enables developers to construct train components using Godot's Node-based system, providing a visual and intuitive approach to train creation and management. Key features include:

- Custom Node-based Classes: Create train elements using our premade and integrated classes based on Godot's familiar Node system.
- Parameter Customization: Fine-tune train parameters directly within the Godot editor.
- Simplified Integration: Seamlessly integrate the MaSzyna simulator physics into your Godot projects.
- Enhanced Debugging: Streamline the debugging process for your train simulations.

### Setup

1. Install Python 3
2. Install CMake 3.30 or newer. CMake will automatically install its dependencies.
3. Clone the repository and checkout submodules

```
git clone <url>
git submodule update --init --recursive
```

### Android development
#### Set up the build system   
For build system setup,
please take a look at [official Godot Engine documentation for Android development](https://docs.godotengine.org/en/4.3/tutorials/export/exporting_for_android.html)
### Compiling
```bash
	cmake -B build-<platform> \
          -DGODOTCPP_TARGET="template_release"
	cmake --build build-<platform>
```

Example:
```bash
	cmake -B build-linux64 \
          -DGODOTCPP_TARGET="template_release"
	cmake --build build-linux64
```

Cross-compiling (for Windows on Linux):
```bash
	cmake -B build-win64 \
          -DGODOTCPP_TARGET="template_release" \
          -DGODOTCPP_PLATFORM=windows \
          -DCMAKE_SYSTEM_NAME=Windows \
          -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
          -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
          -DCMAKE_SIZEOF_VOID_P=8
	cmake --build build-win64
```
### Compatibility

| Plugin Version | Godot Engine version | Windows | Linux | Mac OS | Android                  | iOS | C++ Standard | MaSzyna Version     |
|----------------|----------------------|---------|-------|--------|--------------------------|-----|--------------|---------------------|
| 25.08.2024     | 4.3                  | ✅       | ✅     | ❌      | ✅ (Target API Level: 34) | ❌   | C++ 17       | 24.06               |
| 11.03.2025     | 4.4                  | ✅       | ✅     | ❌      | ✅ (Target API Level: 34) | ❌   | C++ 17       | 24.06               |
| 11.04.2025     | 4.4                  | ✅       | ✅     | ❌      | ✅ (Target API Level: 34) | ❌   | C++ 17       | 24.06               |
| 30.09.2025     | 4.5                  | ✅       | ✅     | ❌      | ✅ (Target API Level: 34) | ❌   | C++ 17       | 24.06               |
| 30.11.2025     | 4.5.x                | ✅       | ✅     | ❌      | ✅ (Target API Level: 34) | ❌   | C++ 17       | 24.06, 25.11        |
| 27.01.2026     | 4.6                  | ✅       | ✅     | ❌      | ✅ (Target API Level: 34) | ❌   | C++ 17       | 24.06, 25.11, 26.01 |
### Documentation 

Project documentation: https://maszyna-reloaded.github.io/MaSzyna-API-wrapper/

If you have found any bug, have a suggestion or want to join us - feel free to open an [issue](https://github.com/MaSzyna-Reloaded/MaSzyna-API-wrapper/issues) or start a [discussion](https://github.com/MaSzyna-Reloaded/MaSzyna-API-wrapper/discussions)!

### Code Quality

#### CI
Clang-tidy checks are performed on CI. Those will fail automatically and publish results if any warning/error is found

### Testing

#### Testing locally

First, ensure you have checked out submodules:

```bash
git submodule update --init
```

Then run tests from the command line (if you have `make` installed, you can use shortcut `make run-tests`):

```bash
godot --path demo --headless -s addons/gut/gut_cmdln.gd -gdir=res://tests/ -gexit
```


Or use Godot Editor, but before that ensure you have configured test dirs.
GUT is storing the config in `user://` filesystem, so you have to do this manually.

![Godot GUT Configuration](docs/assets/gut-gui-tests-config.png)

Then run all tests:

![Godot GUT Running Tests](docs/assets/gut-gui-tests-running.png)


#### Testing using docker

From the command line, build the image first:

```bash
docker build-t godot-tests .
```

then run the image:

```bash
docker run --rm godot-tests
```

If you have `make`, you can simply use the shortcut `make docker-run-tests`.

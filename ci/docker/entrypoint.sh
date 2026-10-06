#!/bin/bash
cd /var/maszyna || exit
# cmake --build with the Makefile generator runs a single job unless told otherwise
export CMAKE_BUILD_PARALLEL_LEVEL="${CMAKE_BUILD_PARALLEL_LEVEL:-$(nproc)}"
while getopts a:p:t:u: flag
do
    # shellcheck disable=SC2220
    case "${flag}" in
        p) platform=${OPTARG};;
        a) arch=${OPTARG};;
        t) target=${OPTARG};;
        u) unit_tests=${OPTARG};;
        *) echo "Invalid option"; exit 1;;
    esac
done
# The image's Godot is built with precision=double, godot-cpp's bundled extension_api.json is single
echo "Dumping extension API from the image's Godot"
mkdir -p build-api/dump && (cd build-api/dump && godot --headless --dump-extension-api) || exit 1
# Replace only on change; a fresh mtime regenerates godot-cpp bindings and rebuilds the whole library
cmp -s build-api/dump/extension_api.json build-api/extension_api.json || \
    mv build-api/dump/extension_api.json build-api/extension_api.json
godotcpp_common_args=(-DGODOTCPP_PRECISION=double -DGODOTCPP_CUSTOM_API_FILE=/var/maszyna/build-api/extension_api.json)

# The host library (template_debug) is what Godot loads to import, test and export the project: the
# CI builds it once (HOST_ONLY=true) and hands it to the other jobs (HOST_PREBUILT=true)
if [ "${HOST_PREBUILT:-}" != "true" ]; then
    echo "Building Dynamic-linked library for host platform"
    cmake -B build-host -DGODOTCPP_TARGET=template_debug "${godotcpp_common_args[@]}" || exit 1
    cmake --build build-host || exit 1
fi
if [ "${HOST_ONLY:-}" = "true" ]; then
    exit 0
fi
if [ "$unit_tests" = "true" ]; then
    echo "Running unit tests..."
    (godot --path demo --headless --import || exit 0) && godot --path demo --headless --import && godot --path demo --headless -s addons/gut/gut_cmdln.gd -gdir=res://tests/ -gexit -gjunit_xml_file=res://test_results.xml || exit 1
    echo "Unit tests passed!"
    exit 0
fi

echo "Creating Dynamic-linked libraries for $target-$platform build..."
case $platform in
  "windows")
    cmake -B build-win64 \
      -DGODOTCPP_TARGET="$target" \
      "${godotcpp_common_args[@]}" \
      -DGODOTCPP_PLATFORM=windows \
      -DCMAKE_SYSTEM_NAME=Windows \
      -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
      -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
      -DCMAKE_SIZEOF_VOID_P=8 || exit 1 #8 for 64bit, 4 for 32
    cmake --build build-win64 || exit 1 ;;
  "linux")
    cmake -B build-linux64 \
      -DGODOTCPP_TARGET="$target" \
      "${godotcpp_common_args[@]}" || exit 1
    cmake --build build-linux64 || exit 1 ;;
  "android")
    cmake -B build-android64 \
      -DGODOTCPP_PLATFORM=android \
      -DANDROID_NDK_ROOT=/usr/lib/android-sdk/ndk/28.1.13356709 \
      -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=24 \
      -DGODOTCPP_TARGET="$target" \
      "${godotcpp_common_args[@]}" || exit 1
    cmake --build build-android64 || exit 1 ;;
esac

mkdir -p "build/${platform}"
export_preset="${platform}_${arch}"
target_file_name="reloaded_${export_preset}"
unzip="true"
if [ "$platform" = "android" ]; then
    unzip="false"
    target_file_name="$target_file_name.apk"
else
    unzip="true"
    target_file_name="$target_file_name.zip"
fi

echo "Importing Godot project..."
(godot --path demo --headless --import || exit 0) && godot --path demo --headless --import
if [ "$target" = "template_release" ]; then
    cd demo && godot --headless --export-release "$export_preset" "../build/${platform}/${target_file_name}" || exit 1
else
    cd demo && godot --headless --export-debug "$export_preset" "../build/${platform}/${target_file_name}" || exit 1
fi

if [ $unzip = "true" ]; then
    cd "../build/${platform}" && unzip "${target_file_name}" && rm "${target_file_name}"
fi

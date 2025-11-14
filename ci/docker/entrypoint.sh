#!/bin/bash
cd /var/maszyna || exit
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
echo "Building Dynamic-linked library for host platform"
scons target="template_debug" || exit 1
if [ "$unit_tests" = "true" ]; then
    echo "Running unit tests..."
    (godot --path demo --headless --import || exit 0) && godot --path demo --headless --import && godot --path demo --headless -s addons/gut/gut_cmdln.gd -gdir=res://tests/ -gexit -gjunit_xml_file=res://test_results.xml
    echo "Unit tests passed!"
    exit 0
fi

echo "Creating Dynamic-linked libraries for $target build..."
scons platform="$platform" arch="$arch" target="$target" || exit 1
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

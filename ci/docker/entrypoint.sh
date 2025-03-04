#!/bin/bash
cd /var/maszyna || exit
while getopts a:p:t: flag
do
    # shellcheck disable=SC2220
    case "${flag}" in
        p) platform=${OPTARG};;
        a) arch=${OPTARG};;
        t) target=${OPTARG};;
    esac
done
echo "Building Dynamic-linked library for host platform"
scons target="template_debug"
echo "Creating Dynamic-linked libraries for $target build..."
scons platform="$platform" arch="$arch" target="$target"
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
(godot --path demo --headless --import || exit 0) &&  godot --path demo --headless --import
if [ "$target" = "template_release" ]; then
    cd demo && godot --headless --export-release "$export_preset" "../build/${platform}/${target_file_name}"
else
    cd demo && godot --headless --export-debug "$export_preset" "../build/${platform}/${target_file_name}"
fi

if [ $unzip = "true" ]; then
    cd "../build/${platform}" && unzip "${target_file_name}" && rm "${target_file_name}"
fi

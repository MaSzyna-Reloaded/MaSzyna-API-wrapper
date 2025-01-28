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
echo "Installing dependencies..."
apt install -y openjdk-17-jdk openjdk-17-jre libncurses5-dev libncursesw5-dev curl unzip zip libssl-dev lzma-dev libxml2-dev zlib1g-dev scons g++-mingw-w64-x86-64 gcc-mingw-w64 git-all build-essential cmake gcc-multilib g++-multilib libc6-dev-i386 g++-mingw-w64-i686
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

if [ "$target" = "template_release" ]; then
    cd demo && godot --headless --export-release "$export_preset" "../build/${platform}/${target_file_name}"
else
    cd demo && godot --headless --export-debug "$export_preset" "../build/${platform}/${target_file_name}"
fi

if [ $unzip = "true" ]; then
    cd "../build/${platform}" && unzip "${target_file_name}" && rm "${target_file_name}"
fi
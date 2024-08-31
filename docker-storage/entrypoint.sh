#!/bin/bash
echo "Creating Dynamic-linked libraries..."
cd /var/maszyna
while getopts a:p: flag
do
    # shellcheck disable=SC2220
    case "${flag}" in
        p) platform=${OPTARG};;
        a) arch=${OPTARG};;
    esac
done
scons platform="$platform" arch="$arch"
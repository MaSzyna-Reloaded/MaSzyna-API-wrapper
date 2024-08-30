#!/bin/bash
echo "Creating Dynamic-linked libraries..."
cd /var/maszyna
#scons platform="android" arch="x86_64"
#scons platform="android" arch="x86_32"
#scons platform="android" arch="arm64"
#scons platform="android" arch="arm32"
scons platform="windows" arch="x86_64"
#scons platform="windows" arch="x86_32"
#scons platform="linux" arch="x86_64"
#scons platform="linux" arch="x86_32"
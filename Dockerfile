# Dockerfile
FROM barichello/godot-ci:4.4

# Set working directory
WORKDIR /app

RUN apt update -y && apt install -y scons g++

COPY godot-cpp godot-cpp
COPY vendor vendor
COPY SConstruct ./SConstruct
RUN scons maszyna_debug=no

# Copy project files
COPY addons ./addons
COPY src ./src
RUN mkdir -p demo/addons demo/bin
RUN scons maszyna_debug=no

COPY demo ./demo

# Run tests
#
# IMPORTANT!
# Godot will only work on the third try :)
# First run ends with segfault, second - will generate caches; and the third one will run tests :)
CMD (godot --path demo --headless --import || exit 0) &&  godot --path demo --headless --import && godot --path demo --headless -s addons/gut/gut_cmdln.gd -gdir=res://tests/ -gexit
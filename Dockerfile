# Dockerfile
FROM barichello/godot-ci:4.3

# Set working directory
WORKDIR /app

RUN apt update -y && apt install -y scons g++
RUN wget https://github.com/godotengine/godot-cpp/archive/refs/tags/godot-4.3-stable.tar.gz
RUN tar -xf godot-4.3-stable.tar.gz
RUN mv godot-cpp-godot-4.3-stable godot-cpp

RUN wget https://github.com/bitwes/Gut/archive/refs/tags/v9.3.0.tar.gz
RUN tar -xf v9.3.0.tar.gz
RUN mkdir vendor && mv Gut-9.3.0 vendor/gut

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
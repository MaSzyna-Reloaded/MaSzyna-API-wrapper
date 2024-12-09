#/bin/bash

while inotifywait -r -e modify,create,delete --exclude '\.swp$|~$' .; do
    find ./src -type f \( -name '*.cpp' -o -name '*.h' \) | grep -q .
    if [ $? -eq 0 ]; then
        echo "Changes detected. Running scons..."
        scons
    fi
done

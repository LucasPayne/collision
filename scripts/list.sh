#!/bin/sh
#
# Print information about source files to build.
#
# Args:
#   <name> dir

if [ $# -ne 1 ] ; then
    echo "give good args"
    exit 1
fi
src_directory=$1

find "$src_directory" -type d | while IFS= read -r directory; do
    if [ -f "$directory/note.txt" ] ; then
        echo $directory
        echo "\t$(cat "$directory/note.txt")"
    fi
done


# Script to generate application/test project dependencies in the form of prerequisites for a make rule.
# This should only be called by a Makefile, e.g. in secondary expansion to valuate
# a command to run test projects.

# Looks like the Makefile calls it, then this is relative to it, already in the root.
# Need a better way to reference project structure.
ROOT=.

if [ $# -ne 1 ] ; then
    echo "ERROR: give good args" 1>&2
    exit 1
fi

name=$1

dir=$(find "src" -type d -name "$name")
if [ "$dir" = "" ] ; then
    echo ____NO_PROJECT # this name is probably never used as a real prerequisite
    exit 0
fi

C_source="$dir/$name.c"
# echo "$C_source"

# Read the C source file and compile the neccessary object linkings
python3 - $C_source << END
import sys
import re
C_source = [line.strip() for line in open(sys.argv[1]).readlines()]
finding = False

for line in C_source:
    if line.endswith("PROJECT_LIBS:"):
        finding = True
    elif finding:
        match = re.match(".*\+ (.*$)", line)
        if match:
            print(f"build/clutter/lib/{match.group(1)}.o ", sep=" ")
        else:
            break
END

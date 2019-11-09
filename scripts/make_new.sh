#!/bin/bash


PROJECT_MAKE_RULE='

PROJECT_NAME: $(SRC_DIR)/PROJECT_NAME/PROJECT_NAME.c $(FILES)
	$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)
do_PROJECT_NAME: PROJECT_NAME
	$(BUILD_DIR)/$<
        
'

if [ $# -ne 3 ] ; then
    echo "give good args"
    exit 1
fi

SRC_DIR=$1
SCHEMATICS_DIR=$2
MAKEFILE=$3

find_schematics(){
    find "$SCHEMATICS_DIR" -type f -name 'schematic_*'
}
schematic_name(){
    # bash does not support nested parameter expansions ...
    # should use awk
    schematic_path=$1
    schematic_filename=$(basename $schematic_path)
    __temp=${schematic_filename%.*}
    schematic=${__temp#schematic_}
    echo "$schematic"
}
    
# https://stackoverflow.com/questions/4726695/bash-and-readline-tab-completion-in-a-user-input-loop
set -o emacs;
tab() {

    line=$READLINE_LINE

    num=0
    last_read=
    # The while loop is executed in a subshell. So any changes you do to the variable will not be available once the subshell exits.
    # ...
    # https://stackoverflow.com/questions/16854280/a-variable-modified-inside-a-while-loop-is-not-remembered
    # find_schematics | while IFS= read -r schematic_path; do

    while IFS= read -r schematic_path ; do
        schematic=$(schematic_name $schematic_path)

        if [[ "$schematic" =~ ^"$line" ]] ; then
            echo -n "$schematic "
            num=$(expr $num + 1)
            last_read="$schematic"
        fi
    done <<< $(find_schematics)

    if [ $num -ne 0 ] ; then
        echo ""
    fi

    if [ $num -eq 1 ] ; then
        READLINE_LINE="$last_read"
    fi

    READLINE_POINT="${#READLINE_LINE}"
}
bind -x '"\t":"tab"';


find_schematics | while IFS= read -r schematic_path; do
    echo -e "\t$(schematic_name $schematic_path)"
done
read -ep "Schematic name: " -r chosen

schematic_path=
schematic=
good=
while IFS= read -r schematic_path; do
    schematic=$(schematic_name $schematic_path)
    if [[ "$chosen" = "$schematic" ]] ; then
        good=Yes
        break
    fi
done <<< "$(find_schematics)"

if [ "$good" = "Yes" ] ; then
    echo "Making project from schematic $schematic"
    echo -e "\t$schematic_path"
    read -ep "Project name: " -r project_name

    echo "Making $schematic $project_name ..."
    proj_dir="$SRC_DIR/$project_name"
    mkdir "$proj_dir"
    cp "$schematic_path" "$proj_dir/$project_name.c"

    printf "$PROJECT_MAKE_RULE" | sed "s/PROJECT_NAME/$project_name/g" >> $MAKEFILE
else   
    echo "Invalid schematic name: $chosen"
fi

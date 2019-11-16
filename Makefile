# Stuff about Makefiles
#--------------------------------------------------------------------------------
# Escaping
# --------
# https://stackoverflow.com/questions/7654386/how-do-i-properly-escape-data-for-a-makefile
# \#
# $$
# Automatic variables:
# --------------------
# Suppose you are writing a pattern rule to compile a ‘.c’ file
# into a ‘.o’ file: how do you write the ‘cc’ command so
# that it operates on the right source file name? You cannot write
# the name in the recipe, because the name is different each time
# the implicit rule is applied.
#
# What you do is use a special feature of make, the automatic
# variables. These variables have values computed afresh for each rule that is
# executed, based on the target and prerequisites of the rule. In this
# example, you would use ‘$@’ for the object file name and ‘$<’
# for the source file name.
#
#
# https://stackoverflow.com/questions/10024279/how-to-use-shell-commands-in-makefile
# each line is run by a separate shell, so this variable
# will not survive to the next line, so you must then use
# it immediately
#
# Shell function:
# https://www.gnu.org/software/make/manual/html_node/Shell-Function.html
# The shell function is unlike any other function other than the wildcard
# function (see The Function wildcard) in that it communicates with the world
# outside of make.
#--------------------------------------------------------------------------------


# Project structure
BUILD_DIR=build
CLUTTER_DIR=build/clutter
LIB_DIR=lib
INCLUDE_DIR=include
SRC_DIR=src
SCRIPTS_DIR=scripts
SCHEMATICS_DIR=utils/schematics
MAKEFILE=Makefile

# GNU make feature:
# Secondary expansions always take place within the scope of the automatic variables
# for that target. This means that you can use variables such
# as $@, $*, etc. during the second expansion and they will
# have their expected values, just as in the recipe. All
# you have to do is defer the expansion by escaping the $.
# ---
# They are expanded at the time make checks the prerequisites of a target.
.SECONDEXPANSION:

# Commands and flags
CC=gcc -rdynamic -Iinclude -Wall
CFLAGS=-lglfw3 -lm -lrt -lm -ldl -lX11 -lpthread -lGL

# General Makefile options
default_target: list

# Scripts
.PHONY .SILENT: list
list:
	echo "make <name> to build, make do_<name> to build and run."
	sh $(SCRIPTS_DIR)/list.sh $(SRC_DIR)

.PHONY .SILENT: new
new: ; $(SCRIPTS_DIR)/make_new.sh $(SRC_DIR) $(SCHEMATICS_DIR) $(MAKEFILE)

# This wasn't actually matching, implicit rule still uses $(CC) and $(CFLAGS) so it can compile the object. Why is it remaking? Because object files
# are deleted.
# %.o: $(LIB_DIR)/%.c
# 	$(CC) -c $< -o $(CLUTTER_DIR)/$@

# %:
# 	 echo $(SRC_DIR)/$@/$@.c $(shell $(SCRIPTS_DIR)/application_dependencies.sh $@)

# This is not using the right clutter directory structure
build/clutter/lib/%.o: lib/%.c
	mkdir -p $(patsubst %.o,%,$@)
	$(CC) -c $< -o $@
%: $(SRC_DIR)/$$@/$$@.c $$(shell $(SCRIPTS_DIR)/application_dependencies.sh $$@)
	$(CC) -o "$(BUILD_DIR)/$@" $^ $(CFLAGS)
	$(BUILD_DIR)/$@

# do_%: $(SRC_DIR)/$$(shell echo $$@ | cut -c 4-)/$$(shell echo $$@ | cut -c 4-).c $$(shell $$(SCRIPTS_DIR)/application_dependencies.sh $$(shell echo $$@ | cut -c 4-))
# 	$(CC) -o "$(BUILD_DIR)/$(shell echo $@ | cut -c 4-)" $^ $(CFLAGS)
# 	$(BUILD_DIR)/$(shell echo $@ | cut -c 4-)

#================================================================================
# Project structure
# =================
# This project is organized into applications and libraries/modules.
# Applications are compiled to [[[ build/applications/ ]]], and modules are compiled
# to [[[ build/lib/ ]]], which reflects the [[[ lib/ ]]] directory but with object files and build
# output.
#
# Applications
# ------------
# These are separate executable projects, no matter the size. They can be quick tests of modules and facilities, areas
# for learning certain things, or just misc. stuff.
# Their source is in the [[[ src/ ]]] directory. This source must contain a central C file with main. Applications are built with
#	$make <application name>,
# which a make rule automatically resolves to a build process.
# Dependencies are given in the source instead of this makefile. The make rule passes the C source through a small utility called
# cslots (install at [[[ utils/cslots/ ]]] and run $cslots --help), which outputs the dependencies given in the file as, e.g.:
# /* 
# PROJECT_LIBS:
# 	+ things_module
# 	- topology
# 	+ camping/fishing
# ...
# */
# Here, build/lib/things_module/things_module.o and build/lib/camping/fishing/fishing.o must be built and linked to the application executable.
# These let you slot in attributes to a source file, used here to slot in project module dependencies.
#
# Project libraries/modules
# -------------------------
# Making an application causes make rules to be run for each project module dependency of the application.
# These make rules are either:
#	- default behaviour.
#		By default, it is assumed the library is a simple C module with a straightforward make process.
#	- sourced from a file named Makefile in the library source directory.
#		This file should contain a build rule for the library object.
#		Its working directory as a sub-make process is [[[ build/lib/<library name>/ ]]], and an
#		exported variable LIB is provided for the [[[ lib/<library name>/ ]]] directory.
#
# Schematic system
# ----------------
# Some base schematics are kept in [[[ utils/schematics/ ]]]. These are used with the command
# 	$make new
# which gives a command line selection of the wanted schematic and name. This creates
# a basic project structure in the [[[ src/ ]]] directory.
#
#
# notes
#--------------------------------------------------------------------------------
# Rebuilding/timing rules not correct.
# Headers not accounted for. May want to rearrange them.
# Variables in recipes seem to be empty.
#================================================================================
BUILD_DIR=build
APPLICATIONS_DIR=build/applications
LIB_DIR=lib
INCLUDE_DIR=include
SRC_DIR=src
SCRIPTS_DIR=scripts
SCHEMATICS_DIR=utils/schematics
MAKEFILE=Makefile

.SECONDEXPANSION:
.EXPORT_ALL_VARIABLES:

# Giving absolute directories as this is important for recursive use of make.
CC=gcc -rdynamic -I$(CURDIR)/include -Wall
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

#================================================================================
# Project library making
#---------------------------------------------------------------------------------
# Recur to sub-Makefiles. These are called Makefile.
# Making is done in the build directory, so these sub-Makefiles should take that into account,
# and can use an exported variable LIB which gives the location of the source files.
# ---- remember headers
#
.PRECIOUS: build/lib/%.o
build/lib/%.o:
	mkdir -p "$(shell dirname "$@")";\
	# echo "!!!!!!!!!! MAKING $@ !!!!!!!!!!!!!!!"
	# sleep 1.5
	test -f "$(CURDIR)/$(subst build/,,$(shell dirname "$@"))/Makefile" && (\
		cd "$(shell dirname "$@")" && LIB=$(CURDIR)/$(subst build/,,$(shell dirname "$@"))\
			$(MAKE) -e --file=$(CURDIR)/$(subst build/,,$(shell dirname "$@"))/Makefile;\
	) || (\
		$(CC) -o $@ -c $(subst build/,,$(@:.o=.c)) $(CFLAGS);\
	)
#================================================================================

#================================================================================
# Application making
#--------------------------------------------------------------------------------
# The cslots utility is used to extract build directives from the source file for the application.
# This is done with secondary expansion, delaying expansions until a target is targetted,
# so the make system can use an implicit dependency graph with an "outgoing nodes" function.
# Example slot name conversion: things/thing -> build/lib/things/thing/thing.o
#
%: $(SRC_DIR)/$$@/$$@.c $$(shell cat $(SRC_DIR)/$$@/$$@.c | cslots PROJECT_LIBS --pattern '$(BUILD_DIR)/$(LIB_DIR)/{n}/{h}.o')
	mkdir -p $(APPLICATIONS_DIR)
	$(CC) -o "$(APPLICATIONS_DIR)/$@" $^ $(CFLAGS)
#================================================================================

.PHONY: clean
clean:
	rm -r build/*

#================================================================================
# TESTS
#--------------------------------------------------------------------------------
.PHONY: t1
t1:
	echo $(shell cat $(SRC_DIR)/text_processing/text_processing.c | cslots PROJECT_LIBS --pattern '$(BUILD_DIR)/$(LIB_DIR)/{n}/{n}.o')

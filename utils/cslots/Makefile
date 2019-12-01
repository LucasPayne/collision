
DEPTH=../..

.DEFAULT_GOAL: help
.PHONY: help
.SILENT: help
help:
	echo "make cslots to compile, sudo make install to install, sudo make all to do it all."

cslots: cslots.l
	flex -o cslots.yy.c $<
	gcc -o $@ cslots.yy.c
.PHONY: t1 t2 t3
t1: cslots
	cat test | ./cslots PROJECT_LIBS --pattern 'lib/{n}/{n}.c'
t2: cslots
	cat test | ./cslots APPLICATION_LIBS -p 'build/{n}/{n}/clutter/{n}.o'
t3: cslots
	cat test | ./cslots APPLICATION_LIBS -p "build/{n}/{h}.o"

.PHONY: clean install all
.SILENT: install all
clean:
	rm *.yy.c
install:
	cp cslots /usr/bin/cslots
	echo "installed cslots to /usr/bin"



all: cslots install

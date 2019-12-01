
DEPTH=../..

application_dependencies: application_dependencies.l
	flex -o application_dependencies.yy.c $<
	gcc -o $@ application_dependencies.yy.c
.PHONY: t1
t1: application_dependencies
	cat $(DEPTH)/src/text_processing/text_processing.c | ./$< $(DEPTH)/src/text_processing

.PHONY: clean
clean:
	rm *.yy.c

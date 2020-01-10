
# This "module" itself has library dependencies. So, build them into the object file.
# note: How could this take into account libraries included from the client application?

interactive_3D.o:
	echo beaned
	sleep 3

# .SECONDEXPANSION:
# interactive_3D.o: $(LIB)/interactive_3D.c $$(shell cat $(LIB)/interactive_3D.c | cslots PROJECT_LIBS --pattern '$(CURDIR)/$(BUILD_DIR)/$(LIB_DIR)/{n}/{h}.o')
# 	$(CC) -o _interactive_3D.o -c $< $(CFLAGS)
# 	ld -relocatable -o $@ _interactive_3D.o $(filter-out $<,$^)

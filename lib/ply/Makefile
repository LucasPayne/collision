ply.o: $(LIB)/ply_stat.l $(LIB)/ply.c
	flex -o ply_stat.yy.c "$<"
	$(CC) -o ply_stat.yy.o -c ply_stat.yy.c $(CFLAGS)
	$(CC) -o _ply.o -c $(LIB)/ply.c $(CFLAGS)
	ar rvs ply.o _ply.o ply_stat.yy.o
	rm ply_stat.yy.c
# not how linker works
# ld -o ply.o _ply.o ply_stat.yy.o -lc
# --- create archive (can it be a .o? if not, should I just make some things a trivial .a library instead of a .o?)
# --- making a .o seems to work, even though it is an archive collection of object files.

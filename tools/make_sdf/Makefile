
CC=gcc -I/usr/local/include/freetype2 -I/usr/include/libpng16
CFLAGS=-lfreetype -lpng

make_sdf: make_sdf.c
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: t1
t1:
	make make_sdf
	./make_sdf -f fonts/cmunbi.ttf -s 16x0 -r 5
.PHONY: clean
clean:
	rm make_sdf


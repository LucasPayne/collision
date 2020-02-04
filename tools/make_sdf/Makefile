
CC=gcc -I/usr/local/include/freetype2 -I/usr/include/libpng16
CFLAGS=-lm -lfreetype -lpng

make_sdf: make_sdf.c
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: t1
t1:
	make make_sdf
	./make_sdf -f fonts/cmunbi.ttf -s 12x0 -r 8 -i
.PHONY: clean
clean:
	rm make_sdf


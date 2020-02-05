
CC=gcc -I/usr/local/include/freetype2 -I/usr/include/libpng16
CFLAGS=-lm -lfreetype -lpng

make_sdf: make_sdf.c
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: t1
t1:
	make make_sdf
	./make_sdf -f fonts/arial_regular.ttf -s 32x0 -r 32 -i
.PHONY: clean
clean:
	rm make_sdf


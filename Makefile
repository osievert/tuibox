all: tuibox

CC=cc

LIBS=-lm
CFLAGS=-O3 -pipe
DEBUGCFLAGS=-Og -pipe -g

.PHONY: tuibox
tuibox:
	$(CC) src/tuibox.c examples/tuibox_basic.c -Iinclude -o examples/tuibox_basic $(LIBS) $(CFLAGS)
	$(CC) src/tuibox.c examples/tuibox_bounce.c -Iinclude -o examples/tuibox_bounce $(LIBS) $(CFLAGS)
	$(CC) src/tuibox.c examples/tuibox_bounce_inline.c -Iinclude -o examples/tuibox_bounce_inline $(LIBS) $(CFLAGS)
	$(CC) src/tuibox.c examples/tuibox_drag.c -Iinclude -o examples/tuibox_drag $(LIBS) $(CFLAGS)

.PHONY: clean
clean:
	rm -f examples/tuibox_basic examples/tuibox_bounce examples/tuibox_bounce_inline examples/tuibox_drag

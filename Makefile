all: tuibox

CC=cc

LIBS=-lm
CFLAGS=-O3 -pipe
DEBUGCFLAGS=-Og -pipe -g

.PHONY: tuibox
tuibox:
	$(CC) examples/tuibox_basic.c -o examples/tuibox_basic $(LIBS) $(CFLAGS)
	$(CC) examples/tuibox_bounce.c -o examples/tuibox_bounce $(LIBS) $(CFLAGS)
	$(CC) examples/tuibox_bounce_inline.c -o examples/tuibox_bounce_inline $(LIBS) $(CFLAGS)
	$(CC) examples/tuibox_drag.c -o examples/tuibox_drag $(LIBS) $(CFLAGS)

.PHONY: clean
clean:
	rm -f examples/tuibox_basic examples/tuibox_bounce examples/tuibox_bounce_inline examples/tuibox_drag

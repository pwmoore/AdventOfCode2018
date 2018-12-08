BIN=Day4
GLIBFLAGS=-I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -lglib-2.0
LIB=../lib/file.c
LIBINC=-I ../lib

$(BIN): $(LIB) $(BIN).c
	cc -ggdb3 $^ $(LIBINC) -lbsd -o $@

.PHONY: clean
clean:
	rm $(BIN)

BIN=Day5
GLIBFLAGS=-I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -lglib-2.0
LIB=../lib/file.c
LIBINC=-I ../lib

$(BIN): $(LIB) $(BIN).c
	cc -O3 $^ $(LIBINC) -lpthread -lbsd -o $@

.PHONY: clean
clean:
	rm $(BIN)

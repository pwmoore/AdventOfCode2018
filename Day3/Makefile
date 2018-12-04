BIN=Day3
GLIBFLAGS=-I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -lglib-2.0

$(BIN): $(BIN).c
	cc $^ -lbsd -o $@

.PHONY: clean
clean:
	rm $(BIN)

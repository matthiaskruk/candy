OBJECTS = main.o can.o ui.o
CFLAGS = -Wall `pkg-config --cflags gtk+-2.0`
LIBS = `pkg-config --libs gtk+-2.0`

all: candy

candy: $(OBJECTS)
	gcc $(CFLAGS) $(LIBS) -o $@ $^

clean:
	rm -rf $(OBJECTS) candy

.PHONY: clean

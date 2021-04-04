CFLAGS   = -g -std=c99 -Wall -O0 -Iglad/
LDFLAGS  = -lm -lglfw -ldl
CC       = cc

example: example.o glad/glad.o
	$(CC) -o $@ example.o glad/glad.o $(LDFLAGS)

example.o: triangulate.h

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f example example.o glad/glad.o

.PHONY: clean

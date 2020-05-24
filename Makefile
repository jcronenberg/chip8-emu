SRCS = $(wildcard *.c)

PROGS = $(patsubst %.c,%,$(SRCS))

all: $(PROGS)

%: %.c
	gcc $(CFLAGS)  -o $@ $< -lGL -lGLU -lglut

clean:
	rm -f $(PROGS)

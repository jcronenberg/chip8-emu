SRCS = $(wildcard *.c)

PROGS = $(patsubst %.c,%,$(SRCS))

all: $(PROGS)

%: %.c
	g++ $(CFLAGS)  -o $@ $< -lm -lGL -lGLU -lglut -lGLEW -lXi -lXmu

clean:
	rm -f $(PROGS)

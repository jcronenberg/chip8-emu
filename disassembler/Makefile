SRCS = $(wildcard *.c)

PROGS = $(patsubst %.c,%,$(SRCS))

all: $(PROGS)

%: %.cpp
	g++ $(CFLAGS)  -o $@ $<

clean:
	rm -f $(PROGS)

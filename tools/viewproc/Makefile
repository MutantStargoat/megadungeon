src = $(wildcard src/*.c)
obj = $(src:.c=.o)
bin = viewproc

CFLAGS = -pedantic -Wall -g
LDFLAGS = -limago

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

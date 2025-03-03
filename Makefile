CFLAGS=-Wall -Werror -Wpedantic

all: 001 002

001:
	@mkdir -p ./bin
	gcc $(CFLAGS) -I. -o ./bin/001_region_alloc ./examples/001_region_alloc.c

002:
	@mkdir -p ./bin
	gcc $(CFLAGS) -I. -o ./bin/002_string ./examples/002_string.c

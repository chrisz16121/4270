all: mu-mips mu-mips_stall

mu-mips: mu-mips-f.c
	gcc -Wall -g -O2 $^ -o $@

mu-mips_stall: mu-mips-s.c
	gcc -Wall -g -O2 $^ -o $@

.PHONY: clean
clean:
	rm -rf *.o *~ mu-mips

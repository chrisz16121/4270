all: mu-mips-assembler mu-mips

mu-mips-assembler: mu-mips-assembler.c
	gcc -Wall -g -O2 $^ -o $@

mu-mips: mu-mips.c
	gcc -Wall -g -O2 $^ -o $@

.PHONY: clean
clean:
	rm -rf *.o *~ mu-mips-assembler
	rm -rf *.o *~ mu-mips
	

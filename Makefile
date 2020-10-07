.phony all:
all: part1.c part2.c part3.c part4.c
	gcc part1.c -o diskinfo
	gcc part2.c -o disklist
	gcc part3.c -o diskget
	gcc part4.c -o diskput

.PHONY clean:
clean:
	-rm -rf *.o *.exe


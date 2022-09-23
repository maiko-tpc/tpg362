all: tpg

tpg: tpg.c
	gcc -o tpg tpg.c

clean:
	rm -f tpg


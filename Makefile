elgamal: elgamal.c
	gcc -o elgamal elgamal.c ec.c -lm -lgmp
clean:
	rm -fr elgamal

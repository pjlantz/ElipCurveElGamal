elgamal: elgamal.c
	gcc -o elgamal elgamal.c -lm -lgmp
clean:
	rm -fr elgamal

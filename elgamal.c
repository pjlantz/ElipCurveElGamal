

/* 
   1) Write down an elliptic curve version of ElGamal
   2) Implement a software library to eprform elliptic curve addition adn doubling over the intergers modulo p.
   3) Implement ElGamal as in 1)
*/



// 1) setup domain parameters q, p , g
// 2) choose public private key pair x and h=g^x

// 3) In order to encrypt a message m
// 3a) generate ephemeral key k, set c1 = g^k and c2 = m*h^k 
// 3b) output ciphertext as c=(c1,c2)
// 4) Recipient can decrypt our ciphertext by computing c2/c1^x

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <fcntl.h>
#include <math.h>


typedef struct elgam_ctx {
  int dom_par_q, dom_par_p, dom_par_g;
  int priv_x, pub_h, eph_k;
} elgam_ctx;


/* Very primitive primality testing. Replace when using bigints */
int is_prime(int n) {
  if(n < 2)
    return 0;
  int i;
  for(i = 2; i < n; i++) {
      if(n % i == 0)
	return 0;
  }
  return 1;
}

int gcd(int a, int b)
{
  int c;
  while (a != 0) {
    c = a; 
    a = b % a;  
    b = c;
  }
  return b;
}


/*
  Sets r to a random GMP integer with the specified number
  of bits.
*/
void get_random_n_bits(mpz_t r, size_t bits)
{
  size_t size = (size_t) ceilf(bits/8);
  char *buffer = (char*) malloc(sizeof(char)*size);
  int prg = open("/dev/random", O_RDONLY);
  read(prg, buffer, size);
  close(prg);
  mpz_import (r, size,1,sizeof(char), 0,0, buffer);
  free(buffer);
}


/*
  Sets r to a random GMP *prime* integer, smaller than max.
*/
void get_random_n_prime(mpz_t r, mpz_t max) {
  do {
    get_random_n_bits(r,mpz_sizeinbase(max,2));
    mpz_nextprime(r,r);
  } while (mpz_cmp(r,max)>=0);
}


/*
  Sets r to a random GMP integer smaller than max.
*/
void get_random_n (mpz_t r, mpz_t max) {
  do {
    get_random_n_bits(r,mpz_sizeinbase(max,2));
  } while (mpz_cmp(r,max)>=0);
  
}

/*
 Init structure. Set domain parameters p, q and g
 */
void init_elgam(elgam_ctx **ectx, size_t bits) {
  *ectx = (elgam_ctx*) malloc(sizeof(elgam_ctx));
  // 1. find large prime p for domain parameter
  mpz_t p, g, x, h;
  mpz_init(p);
  mpz_init(g);
  mpz_init(x);
  mpz_init(h);
  get_random_n_bits(p, bits);
  mpz_nextprime(p, p);
  gmp_printf("p = %Zd\n\n", p);
 
  get_random_n_prime(g, p);
  gmp_printf("g = %Zd\n\n", g);

  get_random_n(x, p);
  gmp_printf("x = %Zd\n\n", x);
  /* h = g^x (mod n) */
  mpz_powm_sec(h, g, x, p);
  gmp_printf("h = %Zd\n\n", h);
 //get_random_n_bits(q, 160);
  //mpz_nextprime(q, q);
  //gmp_printf("q= %Zd\n\n", q);  
  

  //printf("GCD(808,101)=%i\n", gcd(808,101));
  /*while (1) {
    srand(time(NULL));
    p = rand() / 1000000; 
    if (is_prime(p)) {
      break;
    }
  }
  while (1) {
    srand(time(NULL));
    q = rand() / 6400000;
    if (is_prime(q)) {
      if (p % q == q)
//gcd(p-1, q) == q)
	break;
    }
  }
  printf("p = %i\n", p);
  printf("q = %i\n", q);*/
}


void destroy_elgam(elgam_ctx *ectx) {
  if (ectx)
    free(ectx);
}


int main() {
  elgam_ctx *ec;
  init_elgam(&ec, 8);
  //printf("Privkey: %i\n", ec->priv_x);
  destroy_elgam(ec);
}


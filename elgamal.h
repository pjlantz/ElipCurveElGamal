/* 
1) Write down an elliptic curve version of ElGamal
2) Implement a software library to eprform elliptic curve addition adn doubling over the intergers modulo p.
3) Implement ElGamal as in 1)
*/

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <fcntl.h>
#include <math.h>


typedef struct elgam_ctx {
  mpz_t dom_par_q, dom_par_p, dom_par_g;
  mpz_t priv_x, pub_h, eph_k;
} elgam_ctx;


typedef struct ciphertext {
  mpz_t c1, c2;
} ciphertext;


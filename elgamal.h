/* 
Todo
1) Fix memory leaks
	1a) Finish addition2, doubling2 and scalar_mul2
	1b) Fix memory leak here in test_init_elgam, then fix init_elgam_ec
2) Refactor lib: ElGamal and ElGamal-EC (different brainpool parameters) 
3) Optimization: Implement projective coordinates and more efficient scalar multiplication
*/

#include "ec.h"
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


typedef struct cipherec {
	point *c1, *c2;
} cipherec;


typedef struct elgam_ec_ctx {
  mpz_t priv_key, eph_k;
  point *pub_key;
  elliptic_curve *ec;
} elgam_ec_ctx;


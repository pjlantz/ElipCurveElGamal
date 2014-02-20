#include "elgamal.h"


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
void get_random_n_prime(mpz_t r, mpz_t max) 
{
  do {
    get_random_n_bits(r,mpz_sizeinbase(max,2));
    mpz_nextprime(r,r);
  } while (mpz_cmp(r,max)>=0);
}


/*
  Sets r to a random GMP integer smaller than max.
*/
void get_random_n (mpz_t r, mpz_t max) 
{
  do {
    get_random_n_bits(r,mpz_sizeinbase(max,2));
  } while (mpz_cmp(r,max)>=0);
  
}


/*
 Init structure. Set domain parameters p, q and g
 */
void init_elgam(elgam_ctx **ectx, size_t bits) 
{
  *ectx = (elgam_ctx*) malloc(sizeof(elgam_ctx));
  // 1. find large prime p for domain parameter
  mpz_t p, g, x, h;
  mpz_init((*ectx)->dom_par_p);
  mpz_init((*ectx)->dom_par_g);
  mpz_init((*ectx)->priv_x);
  mpz_init((*ectx)->pub_h);
  mpz_init((*ectx)->eph_k);
  get_random_n_bits((*ectx)->dom_par_p, bits);
  mpz_nextprime((*ectx)->dom_par_p, (*ectx)->dom_par_p);
  gmp_printf("\n\np = %Zd\n", (*ectx)->dom_par_p);
 
  get_random_n_prime((*ectx)->dom_par_g, (*ectx)->dom_par_p);
  gmp_printf("g = %Zd\n", (*ectx)->dom_par_g);

  get_random_n((*ectx)->priv_x, (*ectx)->dom_par_p);
  gmp_printf("x = %Zd\n", (*ectx)->priv_x);
  /* h = g^x (mod n) */
  mpz_powm_sec((*ectx)->pub_h, (*ectx)->dom_par_g, (*ectx)->priv_x, (*ectx)->dom_par_p);
  gmp_printf("h = %Zd\n\n", (*ectx)->pub_h);
}


void destroy_elgam(elgam_ctx *ectx) 
{
  if (ectx) {
    mpz_clears(ectx->dom_par_p, ectx->dom_par_g, ectx->dom_par_q, NULL);
    mpz_clears(ectx->priv_x, ectx->pub_h, ectx->eph_k, NULL);
    free(ectx);
    ectx = NULL;
  }
}


void destroy_ciphertxt(ciphertext *ct) 
{
  if (ct) {
    mpz_clears(ct->c1, ct->c2, NULL);
    free(ct);
    ct = NULL;
  }
}


ciphertext* encrypt(mpz_t m, elgam_ctx *ectx) 
{
  ectx->eph_k;
  get_random_n(ectx->eph_k, ectx->dom_par_p);
  ciphertext *ct = malloc(sizeof(ciphertext));
  mpz_init(ct->c1);
  mpz_init(ct->c2);
  mpz_powm_sec(ct->c1, ectx->dom_par_g, ectx->eph_k, ectx->dom_par_p);
  mpz_powm_sec(ct->c2, ectx->pub_h, ectx->eph_k, ectx->dom_par_p);
  mpz_mul(ct->c2, m, ct->c2);
  mpz_mod(ct->c2, ct->c2, ectx->dom_par_p);
  gmp_printf("c1 = %Zd\n", ct->c1);
  gmp_printf("c2 = %Zd\n\n", ct->c2);
  return ct;
}


void decrypt(mpz_t msg, ciphertext *ct, elgam_ctx *ectx) 
{
  mpz_powm_sec(ct->c1, ct->c1, ectx->priv_x, ectx->dom_par_p);
  mpz_invert(ct->c1, ct->c1, ectx->dom_par_p);
  mpz_mul(msg, ct->c2, ct->c1);
  mpz_mod(msg, msg, ectx->dom_par_p);
}


int main() 
{
  mpz_t msg, msg_decr;
  mpz_init(msg);
  mpz_init(msg_decr);
  mpz_set_ui(msg, 100);
  gmp_printf("\nBefore encryption = %Zd\n", msg);
  elgam_ctx *ec;
  ciphertext *ct;

  init_elgam(&ec, 16);
  ct = encrypt(msg, ec);
  decrypt(msg_decr, ct, ec);

  gmp_printf("After decryption= %Zd\n\n", msg_decr);

  mpz_clears(msg_decr, NULL);
  mpz_clears(msg, NULL);
  destroy_ciphertxt(ct);
  destroy_elgam(ec);
}

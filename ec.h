#include <gmp.h>
#include <stdlib.h>


typedef struct point {
  mpz_t x, y;
} point;


typedef struct elliptic_curve {
  mpz_t a, b, p;
  point *base;
} elliptic_curve;


void init_point(point **);
void destroy_point(point *);

point* ecc_scalar_mul(elliptic_curve *, mpz_t, point *);
point* ecc_addition(elliptic_curve *, point *, point *);
point* ecc_doubling(elliptic_curve *, point *);

point* ecc_scalar_mul2(elliptic_curve *, mpz_t, point *);
void ecc_addition2(elliptic_curve *, point *, point *, point *);
void ecc_doubling2(elliptic_curve *, point *, point *);

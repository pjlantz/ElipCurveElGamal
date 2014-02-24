#include "ec.h"


void init_point(point **p) 
{
	*p = (point*) malloc(sizeof(point));
	mpz_init((*p)->x);
	mpz_init((*p)->y);
}


void destroy_point(point *p)
{
	if (p) {
		mpz_clears(p->x, p->y, NULL);
		free(p);
		p = NULL;
	}
}


point* ecc_scalar_mul(elliptic_curve *ec, mpz_t m, point *p) {
	if (mpz_cmp_ui(m, 1) == 0) {
		return p;
	} else if (mpz_even_p(m) == 0) {
		mpz_sub_ui(m, m, 1);
		return ecc_addition(ec, p, ecc_scalar_mul(ec,m,p));
	} else {
		mpz_div_ui(m, m, 2);
		return ecc_scalar_mul(ec, m, ecc_doubling(ec, p));
	}
}


point* ecc_scalar_mul2(elliptic_curve *ec, mpz_t m, point *p) {
	long i, bits;
	point *r, *q, *tmp;

	init_point(&r);
	init_point(&q);
	init_point(&tmp);	
	mpz_set(r->x, p->x);
	mpz_set(r->y, p->y);

	bits = mpz_sizeinbase(m, 2);
	for(i = 0; i < bits; i++) {
		// 2R
		ecc_doubling2(ec, r, tmp);
		mpz_set(r->x, tmp->x);
		mpz_set(r->y, tmp->y);
		gmp_printf("Result 1: %Zd, %Zd\n", r->x, r->y);
		if (mpz_tstbit(m, i) == 1) {
			// Q = Q + R
			ecc_addition2(ec, q, r, tmp);
			//mpz_set(q->x, tmp->x);
			//mpz_set(q->y, tmp->y);			
		}
		gmp_printf("Result 2: %Zd, %Zd\n", q->x, q->y);
	}

	return q;
}


point* ecc_doubling(elliptic_curve *ec, point *p)
{
	point *r = malloc(sizeof(point));
	mpz_init(r->x);
	mpz_init(r->y);
	mpz_mod(p->x, p->x, ec->p);
	mpz_mod(p->y, p->y, ec->p);
	mpz_t temp, slope;
	mpz_init(temp);
	mpz_init_set_ui(slope, 0);

	// temp = 2*y1
	mpz_mul_ui(temp, p->y, 2);
	// temp = temp^-1 mod p
	mpz_invert(temp, temp, ec->p);
	// slope = x1*x1 = x1^2
	mpz_mul(slope, p->x, p->x);
	// slope = slope * 3
	mpz_mul_ui(slope, slope, 3);

	// slope = slope + a
	mpz_add(slope, slope, ec->a);

	// slope = slope * temp (numinator * denuminator)
	mpz_mul(slope, slope, temp);
	// slope = slope mod p
	mpz_mod(slope, slope, ec->p);

	// x3 = slope * slope
	mpz_mul(r->x, slope, slope);
	mpz_sub(r->x, r->x, p->x);
	mpz_sub(r->x, r->x, p->x);
	mpz_mod(r->x, r->x, ec->p);
	mpz_sub(temp, p->x, r->x);
	mpz_mul(r->y, slope, temp);
	mpz_sub(r->y, r->y, p->y);
	mpz_mod(r->y, r->y, ec->p);

	//return r;
	mpz_clears(temp, slope, NULL);
	return r;
}


void ecc_doubling2(elliptic_curve *ec, point *p, point *r)
{
	mpz_init(r->x);
	mpz_init(r->y);
	mpz_mod(p->x, p->x, ec->p);
	mpz_mod(p->y, p->y, ec->p);
	mpz_t temp, slope;
	mpz_init(temp);
	mpz_init_set_ui(slope, 0);

	// temp = 2*y1
	mpz_mul_ui(temp, p->y, 2);
	// temp = temp^-1 mod p
	mpz_invert(temp, temp, ec->p);
	// slope = x1*x1 = x1^2
	mpz_mul(slope, p->x, p->x);
	// slope = slope * 3
	mpz_mul_ui(slope, slope, 3);

	// slope = slope + a
	mpz_add(slope, slope, ec->a);

	// slope = slope * temp (numinator * denuminator)
	mpz_mul(slope, slope, temp);
	// slope = slope mod p
	mpz_mod(slope, slope, ec->p);

	// x3 = slope * slope
	mpz_mul(r->x, slope, slope);
	mpz_sub(r->x, r->x, p->x);
	mpz_sub(r->x, r->x, p->x);
	mpz_mod(r->x, r->x, ec->p);
	mpz_sub(temp, p->x, r->x);
	mpz_mul(r->y, slope, temp);
	mpz_sub(r->y, r->y, p->y);
	mpz_mod(r->y, r->y, ec->p);

	//return r;
	mpz_clears(temp, slope, NULL);
}



point* ecc_addition(elliptic_curve *ec, point *p, point *q)
{
	point *r = malloc(sizeof(point));
	mpz_init(r->x);
	mpz_init(r->y);
	mpz_mod(p->x, p->x, ec->p);
	mpz_mod(p->y, p->y, ec->p);
	mpz_mod(q->x, q->x, ec->p);
	mpz_mod(q->y, q->y, ec->p);
	mpz_t temp,slope;
	mpz_init(temp);
	mpz_init_set_ui(slope, 0);

	//if (mpz_cmp(p->x, q->x) == 0 && mpz_cmp(p->y, q->y) == 0)
	//  return ecc_doubling(ec, p);

	// temp = x1-x2
	mpz_sub(temp, p->x, q->x);
	// temp = temp mod p
	mpz_mod(temp, temp, ec->p);
	// temp^-1 mod p
	mpz_invert(temp, temp, ec->p);
	// slope = y1-y2
	mpz_sub(slope, p->y, q->y);
	// slope = slope * temp
	mpz_mul(slope, slope, temp);
	// slope = slope mod p
	mpz_mod(slope, slope, ec->p);

	// x3 = slope * slope = alpha^2
	mpz_mul(r->x, slope, slope);

	// x3 = x3 - x1
	mpz_sub(r->x, r->x, p->x);
	// x3 = x3 - x2
	mpz_sub(r->x, r->x, q->x);
	// x3 = x3 mod p
	mpz_mod(r->x, r->x, ec->p);

	// temp = x1 - x3
	mpz_sub(temp, p->x, r->x);
	// y3 = slope * temp
	mpz_mul(r->y, slope, temp);
	// y3 = y3 - y1
	mpz_sub(r->y, r->y, p->y);
	// y3 = y3 mod p
	mpz_mod(r->y, r->y, ec->p);
	//return r;
	mpz_clears(temp, slope, NULL);
	return r;
}


void ecc_addition2(elliptic_curve *ec, point *p, point *q, point *r)
{
	mpz_mod(p->x, p->x, ec->p);
	mpz_mod(p->y, p->y, ec->p);
	mpz_mod(q->x, q->x, ec->p);
	mpz_mod(q->y, q->y, ec->p);
	mpz_t temp, slope;
	mpz_init(temp);
	mpz_init_set_ui(slope, 0);

	//if (mpz_cmp(p->x, q->x) == 0 && mpz_cmp(p->y, q->y) == 0)
	//  return ecc_doubling(ec, p);

	// temp = x1-x2
	mpz_sub(temp, p->x, q->x);
	// temp = temp mod p
	mpz_mod(temp, temp, ec->p);
	// temp^-1 mod p
	mpz_invert(temp, temp, ec->p);
	// slope = y1-y2
	mpz_sub(slope, p->y, q->y);
	// slope = slope * temp
	mpz_mul(slope, slope, temp);
	// slope = slope mod p
	mpz_mod(slope, slope, ec->p);

	// x3 = slope * slope = alpha^2
	mpz_mul(r->x, slope, slope);

	// x3 = x3 - x1
	mpz_sub(r->x, r->x, p->x);
	// x3 = x3 - x2
	mpz_sub(r->x, r->x, q->x);
	// x3 = x3 mod p
	mpz_mod(r->x, r->x, ec->p);

	// temp = x1 - x3
	mpz_sub(temp, p->x, r->x);
	// y3 = slope * temp
	mpz_mul(r->y, slope, temp);
	// y3 = y3 - y1
	mpz_sub(r->y, r->y, p->y);
	// y3 = y3 mod p
	mpz_mod(r->y, r->y, ec->p);
	//return r;
	mpz_clears(temp, slope, NULL);
}



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
	mpz_import (r, size, 1, sizeof(char), 0, 0, buffer);
	free(buffer);
}


/*
  Sets r to a random GMP *prime* integer, smaller than max.
*/
void get_random_n_prime(mpz_t r, mpz_t max) 
{
	do {
		get_random_n_bits(r, mpz_sizeinbase(max, 2));
		mpz_nextprime(r, r);
	} while (mpz_cmp(r, max) >= 0);
}


/*
  Sets r to a random GMP integer smaller than max.
*/
void get_random_n(mpz_t r, mpz_t max) 
{
	do {
		get_random_n_bits(r, mpz_sizeinbase(max, 2));
	} while (mpz_cmp(r, max) >= 0);
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


/* setup elliptic curve, public and private key
 Using the brainpoolP160r1 - EC domain parameters
 http://www.ecc-brainpool.org/download/Domain-parameters.pdf
 */
void init_elgam_ec(elgam_ec_ctx **eec_ctx)
{
    *eec_ctx = (elgam_ec_ctx*) malloc(sizeof(elgam_ec_ctx));
    elliptic_curve *ecc = malloc(sizeof(elliptic_curve));
    (*eec_ctx)->ec = ecc;

    mpz_set_str(ecc->a, "340E7BE2A280EB74E2BE61BADA745D97E8F7C300", 16); 
    mpz_set_str(ecc->b, "1E589A8595423412134FAA2DBDEC95C8D8675E58", 16); 
    mpz_set_str(ecc->p, "E95E4A5F737059DC60DFC7AD95B3D8139515620F", 16); 

	mpz_init((*eec_ctx)->priv_key);
	init_point(&(ecc->base));
	init_point(&((*eec_ctx)->pub_key));

	mpz_set_str(ecc->base->x, "BED5AF16EA3F6A4F62938C4631EB5AF7BDBCDBC3", 16); 
	mpz_set_str(ecc->base->y, "1667CB477A1A8EC338F94741669C976316DA6321", 16); 
	gmp_printf("\np = %Zd\n", ecc->p);
	get_random_n((*eec_ctx)->priv_key, ecc->p);
	gmp_printf("x = %Zd\n", (*eec_ctx)->priv_key);

	mpz_t tmp;
	mpz_init_set(tmp, (*eec_ctx)->priv_key);
	(*eec_ctx)->pub_key = ecc_scalar_mul((*eec_ctx)->ec, tmp, ecc->base);
	mpz_clears(tmp, NULL);
	gmp_printf("Base point P = (%Zd,%Zd)\n", ecc->base->x, ecc->base->y);
	gmp_printf("Public key xP =  (%Zd,%Zd)\n\n", ((*eec_ctx)->pub_key)->x, ((*eec_ctx)->pub_key)->y);
}


void test_init_elgam_ec(elgam_ec_ctx **eec_ctx)
{
    *eec_ctx = (elgam_ec_ctx*) malloc(sizeof(elgam_ec_ctx));
    elliptic_curve *ecc = malloc(sizeof(elliptic_curve));
    (*eec_ctx)->ec = ecc;

	mpz_init_set_ui(ecc->a, 1);
	mpz_init_set_ui(ecc->b, 3);
	mpz_init_set_ui(ecc->p, 23);

	mpz_init((*eec_ctx)->priv_key);
	init_point(&(ecc->base));
	init_point(&((*eec_ctx)->pub_key));

	//mpz_init_set_ui(ecc->base->x, 21);
	//mpz_init_set_ui(ecc->base->y, 1);
}


void destroy_elgam_ec(elgam_ec_ctx *eec_ctx) 
{
	if (eec_ctx) {
 		mpz_clears(eec_ctx->priv_key, eec_ctx->eph_k, NULL);
		mpz_clears(eec_ctx->ec->a, eec_ctx->ec->b, eec_ctx->ec->p, NULL);
		destroy_point(eec_ctx->ec->base);
		destroy_point(eec_ctx->pub_key);
		if (eec_ctx->ec) {
			free(eec_ctx->ec);
			eec_ctx->ec = NULL;
		}
		free(eec_ctx);
		eec_ctx = NULL;
	}
}


cipherec* encrypt_ec(elgam_ec_ctx *eec, point *pm)
{
	gmp_printf("Encrypted: (%Zd,%Zd)\n", pm->x, pm->y);  

	mpz_init(eec->eph_k);
	get_random_n(eec->eph_k, eec->ec->p);
	gmp_printf("\nEphemeral key = %Zd\n", eec->eph_k);

	cipherec *cipher = malloc(sizeof(cipherec));
	init_point(&cipher->c1);
	init_point(&cipher->c2);
	mpz_t tmp;
	mpz_init_set(tmp, eec->eph_k);
	cipher->c1 = ecc_scalar_mul(eec->ec, tmp, eec->ec->base);
	mpz_clears(tmp, NULL);

	mpz_init_set(tmp, eec->eph_k);
	cipher->c2 = ecc_scalar_mul(eec->ec, tmp, eec->pub_key);
	mpz_clears(tmp, NULL);
	gmp_printf("Cipher c1: (%Zd,%Zd)\n", cipher->c1->x, cipher->c1->y);
	gmp_printf("Cipher c2 without msg: (%Zd,%Zd)\n", cipher->c2->x, cipher->c2->y);
	cipher->c2 = ecc_addition(eec->ec, cipher->c2, pm);
	gmp_printf("Cipher c2 with msg: (%Zd,%Zd)\n", cipher->c2->x, cipher->c2->y);
	mpz_clears(eec->eph_k, NULL);
	return cipher;
}


point* decrypt_ec(elgam_ec_ctx *eec, cipherec *c)
{
  	point *d1, *d2;
  	init_point(&d1);
  	init_point(&d2);
	mpz_t tmp;
  	mpz_init_set(tmp, eec->priv_key);
  	d1 = ecc_scalar_mul(eec->ec, tmp, c->c1);

  	mpz_clears(tmp, NULL);
  	gmp_printf("D1=(%Zd,%Zd)\n", d1->x, d1->y);
	gmp_printf("Before neg: (%Zd,%Zd)\n", d1->x, d1->y);
	mpz_neg(d1->y, d1->y);
  	gmp_printf("After neg: (%Zd,%Zd)\n", d1->x, d1->y);
  	d2 = ecc_addition(eec->ec, c->c2, d1);
  	gmp_printf("Decrypted: (%Zd,%Zd)\n", d2->x, d2->y);
	destroy_point(d1);
	return d2;
}


void destroy_cipherec(cipherec *c)
{
	if (c) {
		destroy_point(c->c1);
		destroy_point(c->c2);
		free(c);
		c = NULL;
	}
}


void test() {

	elgam_ec_ctx *eec;
	test_init_elgam_ec(&eec);
	// P + Q = R = (5,15)  
	point *p, *q;
	init_point(&p);
	init_point(&q);
	mpz_set_ui(p->x, 10);
	mpz_set_ui(p->y, 1);
	mpz_set_ui(q->x, 21);
	mpz_set_ui(q->y, 4);

	point *r;
	init_point(&r);
	point *r2;
	init_point(&r);	
	ecc_addition2(eec->ec, p, q, r);
	gmp_printf("Addition R=(%Zd, %Zd)\n", r->x, r->y);

	ecc_doubling2(eec->ec, p, r);
	// 2P = R = (4,5)
	gmp_printf("R=(%Zd, %Zd)\n", r->x, r->y);
	// 4P = R = (19,2)
	// 3P = R = (12,8)
	mpz_t m;
	mpz_init(m);
	mpz_set_ui(m, 4);

	r2 = ecc_scalar_mul2(eec->ec, m, p); 
	gmp_printf("R=(%Zd, %Zd)\n", r2->x, r2->y);

	// free up stuff
	mpz_clears(m, NULL);

	destroy_point(p);
	destroy_point(q);
	destroy_point(r);
	destroy_point(r2);
	destroy_elgam_ec(eec);

}


int main() 
{
	//test();
	// ElGamal-EC
	elgam_ec_ctx *eec;
	init_elgam_ec(&eec);

	point *p;
	cipherec *c;
	init_point(&p);
	mpz_init_set_ui(p->x, 666);
	mpz_init_set_ui(p->y, 123);
	// c1 = kP (rand k * base point)
	// c2 = xkP + Pm (public key xP * rand k) + point on curve (secret message)
	c = encrypt_ec(eec, p);
	destroy_point(p);
	
	init_point(&p);
	// c1 * x = c1' = xkP
	// Pm = c1' - c2 = xkP - xkP + Pm
	p = decrypt_ec(eec, c);
	destroy_point(p);
	destroy_cipherec(c);

	destroy_elgam_ec(eec);

	// ElGamal
	/*mpz_t msg, msg_decr;
	mpz_init(msg);
	mpz_init(msg_decr);
	mpz_set_ui(msg, 100);
	gmp_printf("\nBefore encryption = %Zd\n", msg);
	elgam_ctx *ec;
	ciphertext *ct;

	init_elgam(&ec, 1024);
	ct = encrypt(msg, ec);
	decrypt(msg_decr, ct, ec);

	gmp_printf("After decryption= %Zd\n\n", msg_decr);

	mpz_clears(msg_decr, NULL);
	mpz_clears(msg, NULL);
	destroy_ciphertxt(ct);
	destroy_elgam(ec);*/
}




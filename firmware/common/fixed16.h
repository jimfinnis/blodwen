/**
 * \file 
 * 16-bit fixed point.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __FIXED16_H
#define __FIXED16_H

#include <stdint.h>

typedef uint16_t fixed16;
typedef uint32_t fixed16d;

#define FIXED16_WBITS	8
#define FIXED16_FBITS (16-FIXED16_WBITS)

#define FIXED16_MASK (((fixed16)1 << FIXED16_FBITS)-1)

#define fixed16_rconst(R) ((fixed16)((R) * (((fixed16d)1 << FIXED16_FBITS) \
	+ ((R) >= 0 ? 0.5 : -0.5))))
#define fixed16_fromint(I) ((fixed16d)(I) << FIXED16_FBITS)
#define fixed16_toint(F) ((F) >> FIXED16_FBITS)
#define fixed16_add(A,B) ((A) + (B))
#define fixed16_sub(A,B) ((A) - (B))
#define fixed16_xmul(A,B)						\
	((fixed16)(((fixed16d)(A) * (fixed16d)(B)) >> FIXED16_FBITS))
#define fixed16_xdiv(A,B)						\
	((fixed16)(((fixed16d)(A) << FIXED16_FBITS) / (fixed16d)(B)))
#define fixed16_fracpart(A) ((fixed16)(A) & FIXED16_FMASK)

#define FIXED16_ONE	((fixed16)((fixed16)1 << FIXED16_FBITS))
#define FIXED16_ONE_HALF (FIXED16_ONE >> 1)
#define FIXED16_TWO	(FIXED16_ONE + FIXED16_ONE)
#define FIXED16_PI	fixed16_rconst(3.14159265358979323846)
#define FIXED16_TWO_PI	fixed16_rconst(2 * 3.14159265358979323846)
#define FIXED16_HALF_PI	fixed16_rconst(3.14159265358979323846 / 2)
#define FIXED16_E	fixed16_rconst(2.7182818284590452354)

#define fixed16_abs(A) ((A) < 0 ? -(A) : (A))


/* Multiplies two fixed16 numbers, returns the result. */
static inline fixed16
fixed16_mul(fixed16 A, fixed16 B)
{
	return (((fixed16d)A * (fixed16d)B) >> FIXED16_FBITS);
}


/* Divides two fixed16 numbers, returns the result. */
static inline fixed16
fixed16_div(fixed16 A, fixed16 B)
{
	return (((fixed16d)A << FIXED16_FBITS) / (fixed16d)B);
}

#endif /* __FIXED16_H */

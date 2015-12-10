#ifndef AAMATH_H
#define AAMATH_H

#include "aatypes.h"
#include <math.h>

#define EPSILON         1.0e-6f
#define PI              3.14159265358979323846264338327950288f
#define HALFPI          1.57079632679489661923132169163975144f
#define TWOPI           6.28318530717958647692528676655900576f
#define TAU             6.28318530717958647692528676655900576f
#define ONEOVERPI       0.318309886183790671537767526745028724f;
#define ONEOVERTWOPI    0.159154943091895335768883763372514362f;
#define ONEOVERTAU      0.159154943091895335768883763372514362f;

#undef AAMATH_APPROXIMATE

union intfloat
{
    r32 f;
    u32 u;
};

// NOTE: see https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Approximations_that_depend_on_the_floating_point_representation
inline r32 AASqrt(r32 x)
{
#if defined(AAMATH_APPROXIMATE)
    intfloat y = {x};
    y.u -= 1 << 23;
    y.u >>= 1;
    y.u += 1 << 29;
    return y.f;
#else
    return sqrtf(x);
#endif
}

// NOTE: see http://rrrola.wz.cz/inv_sqrt.html - improved fast inv sqrt
inline r32 InvSqrt(r32 x)
{
#if defined(AAMATH_APPROXIMATE)
    intfloat y = {x};
    y.u = 0x5F1FFFF9ul - (y.u >> 1);
    return 0.703952253f * y.f * (2.38924456f - x * y.f * y.f);

//    r32 halfx = 0.5f * x;
//    int i = *(int*)&x;
//    i = 0x5f3759df - (i >> 1);
//    x = *(r32*)&i;
//    x = x * (1.5f - halfx * x * x);
//    return x;

#else
    return 1.0f / sqrtf(x);
#endif
}

// NOTE: check if two floats are relatively equal
inline b32 AreEqual(r32 a, r32 b, r32 epsilon = EPSILON)
{
    return fabs(a - b) <= epsilon * (fabs(a) + fabs(b) + 1.0f);
}

// NOTE: check if float is within epsilon of zero
inline b32 IsZero(r32 x, r32 epsilon = EPSILON)
{
    return fabs(x) <= epsilon;
}

inline void SinCos(r32 angle, r32 &a, r32 &b)
{
    a = sinf(angle);
    b = cosf(angle);
}

#endif


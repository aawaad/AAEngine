#ifndef VEC3_H
#define VEC3_H

#include "aamath.h"

union v3
{
    struct
    {
        r32 x, y, z;
    };
    struct
    {
        r32 u, v, w;
    };
    struct
    {
        r32 r, g , b;
    };
    r32 E[3];
};

//
// NOTE: Static constants
//

static const v3 VEC3_XAXIS = {1.0f, 0, 0};
static const v3 VEC3_YAXIS = {0, 1.0f, 0};
static const v3 VEC3_ZAXIS = {0, 0, 1.0f};
static const v3 VEC3_ORIGIN = {0, 0, 0};

//
// NOTE: Operators
//

inline v3 operator-(const v3 &v)
{
    v3 result;

    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;

    return result;
}

inline v3 operator+(const v3 &a, const v3 &b)
{
    v3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

inline v3 operator+(const v3 &a, const r32 &b)
{
    v3 result;

    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;

    return result;
}

inline v3 operator+(const r32 &b, const v3 &a)
{
    v3 result;

    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;

    return result;
}

inline v3 operator-(const v3 &a, const v3 &b)
{
    v3 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

inline v3 operator-(const v3 &a, const r32 &b)
{
    v3 result;

    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;

    return result;
}

inline v3 operator-(const r32 &b, const v3 &a)
{
    v3 result;

    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;

    return result;
}

inline v3 operator*(const v3 &a, r32 b)
{
    v3 result;

    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

inline v3 operator*(r32 a, const v3 &b)
{
    v3 result = b * a;

    return result;
}

inline v3 operator/(const v3 &a, r32 b)
{
    v3 result;

    r32 oneOverB = 1.0f / b;

    result.x = a.x * oneOverB;
    result.y = a.y * oneOverB;
    result.z = a.z * oneOverB;

    return result;
}

inline v3 operator/(r32 a, const v3 &b)
{
    v3 result;

    r32 oneOverA = 1.0f / a;

    result.x = b.x * oneOverA;
    result.y = b.y * oneOverA;
    result.z = b.z * oneOverA;

    return result;
}


inline v3 &operator*=(v3 &a, const r32 &b)
{
    a = a * b;

    return a;
}

inline v3 &operator/=(v3 &a, const r32 &b)
{
    a = a / b;

    return a;
}

inline v3 &operator+=(v3 &a, const v3 &b)
{
    a = a + b;

    return a;
}

inline v3 &operator+=(v3 &a, const r32 &b)
{
    a = a + b;

    return a;
}

inline v3 &operator-=(v3 &a, const v3 &b)
{
    a = a - b;

    return a;
}

inline v3 &operator-=(v3 &a, const r32 &b)
{
    a = a - b;

    return a;
}
 
inline b32 operator==(const v3 &a, const v3 &b)
{
    return (AreEqual(a.x, b.x)
            && AreEqual(a.y, b.y)
            && AreEqual(a.z, b.z));
}
 
inline b32 operator!=(const v3 &a, const v3 &b)
{
    return !(AreEqual(a.x, b.x)
            && AreEqual(a.y, b.y)
            && AreEqual(a.z, b.z));
}

//
// NOTE: Functions
//

inline v3 Cross(const v3 &a, const v3 &b)
{
    v3 result = {a.y * b.z + a.z * b.y,
                 a.z * b.x + a.x * b.z,
                 a.x * b.y + b.x * a.y};

    return result;
}

inline r32 DistanceSq(const v3 &a, const v3 &b)
{
    r32 x = b.x - a.x,
        y = b.y - a.y,
        z = b.z - a.z;

    return (x*x + y*y + z*z);
}

inline r32 Distance(const v3 &a, const v3 &b)
{
    return AASqrt(DistanceSq(a, b));
}

inline r32 Dot(const v3 &a, const v3 &b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

inline v3 Hadamard(const v3 &a, const v3 &b)
{
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

inline r32 LengthSq(const v3 &v)
{
    return Dot(v, v);
}

inline r32 Length(const v3 &v)
{
    return AASqrt(LengthSq(v));
}

inline void Normalize(v3 &v)
{
    r32 oneOverLength = InvSqrt(LengthSq(v));

    v.x *= oneOverLength;
    v.y *= oneOverLength;
    v.z *= oneOverLength;
}

inline v3 Reflect(const v3 &v, const v3 &n)
{
    return v - 2.0f * (Dot(v, n) * n);
}

inline v3 Refract(const v3 &v, const v3 &n, r32 idx)
{
    r32 ndotv = Dot(n, v);
    r32 k = 1.0f - idx * idx * (1.0f - ndotv * ndotv);
    if (k < 0)
        return {0, 0};
    else
        return idx * v - (idx * ndotv + AASqrt(k)) * n;
}

// NOTE: A . (B x C)
inline r32 TripleScal(const v3 &a, const v3 &b, const v3 &c)
{
    return Dot(a, Cross(b, c));
}

// NOTE: (B x C) x A
inline v3 TripleVec(const v3 &a, const v3 &b, const v3 &c)
{
    return Cross(a, Cross(b, c));
}

inline b32 IsZero(const v3 &v)
{
    return (LengthSq(v) <= EPSILON);
}

inline b32 IsUnit(const v3 &v)
{
    return IsZero(1.0f - v.x * v.x - v.y * v.y - v.z * v.z);
}

//
// NOTE: v3 signed int
//

union v3s
{
    struct
    {
        s32 x, y, z;
    };
    struct
    {
        s32 u, v, w;
    };
    struct
    {
        s32 r, g , b;
    };
    s32 E[3];
};

//
// NOTE: Static constants
//

static const v3s VEC3S_XAXIS = {1, 0, 0};
static const v3s VEC3S_YAXIS = {0, 1, 0};
static const v3s VEC3S_ZAXIS = {0, 0, 1};
static const v3s VEC3S_ORIGIN = {0, 0, 0};

//
// NOTE: Operators
//

inline v3s operator-(const v3s &v)
{
    v3s result;

    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;

    return result;
}

inline v3s operator+(const v3s &a, const v3s &b)
{
    v3s result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

inline v3s operator+(const v3s &a, const s32 &b)
{
    v3s result;

    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;

    return result;
}

inline v3s operator+(const s32 &b, const v3s &a)
{
    v3s result;

    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;

    return result;
}

inline v3s operator-(const v3s &a, const v3s &b)
{
    v3s result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

inline v3s operator-(const v3s &a, const s32 &b)
{
    v3s result;

    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;

    return result;
}

inline v3s operator-(const s32 &b, const v3s &a)
{
    v3s result;

    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;

    return result;
}

inline v3s operator*(const v3s &a, s32 b)
{
    v3s result;

    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

inline v3s operator*(s32 a, const v3s &b)
{
    v3s result = b * a;

    return result;
}

inline v3s operator/(const v3s &a, s32 b)
{
    v3s result;

    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;

    return result;
}

inline v3s operator/(s32 a, const v3s &b)
{
    v3s result;

    result.x = b.x / a;
    result.y = b.y / a;
    result.z = b.z / a;

    return result;
}


inline v3s &operator*=(v3s &a, const s32 &b)
{
    a = a * b;

    return a;
}

inline v3s &operator/=(v3s &a, const s32 &b)
{
    a = a / b;

    return a;
}

inline v3s &operator+=(v3s &a, const v3s &b)
{
    a = a + b;

    return a;
}

inline v3s &operator+=(v3s &a, const s32 &b)
{
    a = a + b;

    return a;
}

inline v3s &operator-=(v3s &a, const v3s &b)
{
    a = a - b;

    return a;
}

inline v3s &operator-=(v3s &a, const s32 &b)
{
    a = a - b;

    return a;
}
 
inline b32 operator==(const v3s &a, const v3s &b)
{
    return ((a.x == b.x) && (a.y  == b.y) && (a.z == b.z));
}
 
inline b32 operator!=(const v3s &a, const v3s &b)
{
    return !((a.x == b.x) && (a.y  == b.y) && (a.z == b.z));
}

//
// NOTE: Functions
//

inline v3s Cross(const v3s &a, const v3s &b)
{
    v3s result = {a.y * b.z + a.z * b.y,
                 a.z * b.x + a.x * b.z,
                 a.x * b.y + b.x * a.y};

    return result;
}

inline s32 DistanceSq(const v3s &a, const v3s &b)
{
    s32 x = b.x - a.x,
        y = b.y - a.y,
        z = b.z - a.z;

    return (x*x + y*y + z*z);
}

inline r32 Distance(const v3s &a, const v3s &b)
{
    return AASqrt((r32)DistanceSq(a, b));
}

inline s32 Dot(const v3s &a, const v3s &b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

inline v3s Hadamard(const v3s &a, const v3s &b)
{
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

inline s32 LengthSq(const v3s &v)
{
    return Dot(v, v);
}

inline r32 Length(const v3s &v)
{
    return AASqrt((r32)LengthSq(v));
}

// NOTE: A . (B x C)
inline s32 TripleScal(const v3s &a, const v3s &b, const v3s &c)
{
    return Dot(a, Cross(b, c));
}

// NOTE: (B x C) x A
inline v3s TripleVec(const v3s &a, const v3s &b, const v3s &c)
{
    return Cross(a, Cross(b, c));
}

inline b32 IsZero(const v3s &v)
{
    return ((v.x == 0) && (v.y == 0) && (v.z == 0));
}

//
// NOTE: v3 unsigned int
//

union v3u
{
    struct
    {
        u32 x, y, z;
    };
    struct
    {
        u32 u, v, w;
    };
    struct
    {
        u32 r, g , b;
    };
    u32 E[3];
};

//
// NOTE: Static constants
//

static const v3u VEC3U_XAXIS = {1, 0, 0};
static const v3u VEC3U_YAXIS = {0, 1, 0};
static const v3u VEC3U_ZAXIS = {0, 0, 1};
static const v3u VEC3U_ORIGIN = {0, 0, 0};

//
// NOTE: Operators
//

inline v3u operator+(const v3u &a, const v3u &b)
{
    v3u result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

inline v3u operator+(const v3u &a, const u32 &b)
{
    v3u result;

    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;

    return result;
}

inline v3u operator+(const u32 &b, const v3u &a)
{
    v3u result;

    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;

    return result;
}

inline v3u operator-(const v3u &a, const v3u &b)
{
    v3u result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

inline v3u operator-(const v3u &a, const u32 &b)
{
    v3u result;

    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;

    return result;
}

inline v3u operator-(const u32 &b, const v3u &a)
{
    v3u result;

    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;

    return result;
}

inline v3u operator*(const v3u &a, u32 b)
{
    v3u result;

    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

inline v3u operator*(u32 a, const v3u &b)
{
    v3u result = b * a;

    return result;
}

inline v3u operator/(const v3u &a, u32 b)
{
    v3u result;

    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;

    return result;
}

inline v3u operator/(u32 a, const v3u &b)
{
    v3u result;

    result.x = b.x / a;
    result.y = b.y / a;
    result.z = b.z / a;

    return result;
}


inline v3u &operator*=(v3u &a, const u32 &b)
{
    a = a * b;

    return a;
}

inline v3u &operator/=(v3u &a, const u32 &b)
{
    a = a / b;

    return a;
}

inline v3u &operator+=(v3u &a, const v3u &b)
{
    a = a + b;

    return a;
}

inline v3u &operator+=(v3u &a, const u32 &b)
{
    a = a + b;

    return a;
}

inline v3u &operator-=(v3u &a, const v3u &b)
{
    a = a - b;

    return a;
}

inline v3u &operator-=(v3u &a, const u32 &b)
{
    a = a - b;

    return a;
}
 
inline b32 operator==(const v3u &a, const v3u &b)
{
    return ((a.x == b.x) && (a.y  == b.y) && (a.z == b.z));
}
 
inline b32 operator!=(const v3u &a, const v3u &b)
{
    return !((a.x == b.x) && (a.y  == b.y) && (a.z == b.z));
}

//
// NOTE: Functions
//

inline u32 Dot(const v3u &a, const v3u &b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

inline v3u Hadamard(const v3u &a, const v3u &b)
{
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

inline b32 IsZero(const v3u &v)
{
    return ((v.x == 0) && (v.y == 0) && (v.z == 0));
}

#endif


#ifndef MAT3_H
#define MAT3_H

#include "aamath.h"
#include "quat.h"

union mat3
{
    struct
    {
        r32 xx, xy, xz,
            yx, yy, yz,
            zx, zy, zz;
    };
    struct
    {
        v3 x, y, z;
    };
    r32 m[9];
};
//
// NOTE: Static constants
//

static const mat3 MAT3_IDENTITY = {1.0f, 0.0f, 0.0f,
                                   0.0f, 1.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f};

//
// NOTE: Operators
//

inline mat3 operator-(const mat3 &a)
{
    mat3 result;

    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;

    return result;
}

inline mat3 operator+(const mat3 &a, const r32 &b)
{
    mat3 result;

    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;

    return result;
}

inline mat3 operator+(const r32 &b, const mat3 &a)
{
    mat3 result;

    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;

    return result;
}

inline mat3 operator+(const mat3 &a, const mat3 &b)
{
    mat3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

inline mat3 operator-(const mat3 &a, const r32 &b)
{
    mat3 result;

    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;

    return result;
}

inline mat3 operator-(const r32 &b, const mat3 &a)
{
    mat3 result;

    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;

    return result;
}

inline mat3 operator-(const mat3 &a, const mat3 &b)
{
    mat3 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

inline mat3 operator*(const mat3 &a, const r32 &b)
{
    mat3 result;

    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

inline mat3 operator*(const r32 &b, const mat3 &a)
{
    mat3 result;

    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

// NOTE: row vector * matrix
inline v3 operator*(const v3 &a, const mat3 &b)
{
    v3 result;

    result.x = a.x * b.xx + a.y * b.yx + a.z * b.zx;
    result.y = a.x * b.xy + a.y * b.yy + a.z * b.zy;
    result.z = a.x * b.xz + a.y * b.yz + a.z * b.zz;

    return result;
}

// NOTE: matrix * column vector
inline v3 operator*(const mat3 &a, const v3 &b)
{
    v3 result;

    result.x = a.xx * b.x + a.xy * b.y + a.xz * b.z;
    result.y = a.yx * b.x + a.yy * b.y + a.yz * b.z;
    result.z = a.zx * b.x + a.zy * b.y + a.zz * b.z;

    return result;
}

inline mat3 operator*(const mat3 &a, const mat3 &b)
{
    mat3 result;

    result.xx = a.xx * b.xx + a.xy * b.yx + a.xz * b.zx;
    result.xy = a.xx * b.xy + a.xy * b.yy + a.xz * b.zy;
    result.xz = a.xx * b.xz + a.xy * b.yz + a.xz * b.zz;

    result.yx = a.yx * b.xx + a.yy * b.yx + a.yz * b.zx;
    result.yy = a.yx * b.xy + a.yy * b.yy + a.yz * b.zy;
    result.yz = a.yx * b.xz + a.yy * b.yz + a.yz * b.zz;

    result.zx = a.zx * b.xx + a.zy * b.yx + a.zz * b.zx;
    result.zy = a.zx * b.xy + a.zy * b.yy + a.zz * b.zy;
    result.zz = a.zx * b.xz + a.zy * b.yz + a.zz * b.zz;

    return result;
}

inline mat3 operator/(const mat3 &a, const r32 &b)
{
    mat3 result;

    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;

    return result;
}

inline mat3 operator/(const r32 &b, const mat3 &a)
{
    mat3 result;

    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;

    return result;
}

inline mat3 &operator+=(mat3 &a, const r32 &b)
{
    a.x = a.x + b;
    a.y = a.y + b;
    a.z = a.z + b;

    return a;
}

inline mat3 &operator+=(mat3 &a, const mat3 &b)
{
    a.x = a.x + b.x;
    a.y = a.y + b.y;
    a.z = a.z + b.z;

    return a;
}

inline mat3 &operator-=(mat3 &a, const r32 &b)
{
    a.x -= b;
    a.y -= b;
    a.z -= b;

    return a;
}

inline mat3 &operator-=(mat3 &a, const mat3 &b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;

    return a;
}

inline mat3 &operator*=(mat3 &a, const r32 &b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;

    return a;
}

// NOTE: row vector *= matrix
inline v3 &operator*=(v3 &a, const mat3 &b)
{
    a = a * b;

    return a;
}

inline mat3 &operator*=(mat3 &a, const mat3 &b)
{
    a = a * b;

    return a;
}

inline mat3 &operator/=(mat3 &a, const r32 &b)
{
    a = a / b;

    return a;
}

inline b32 operator==(const mat3 &a, const mat3 &b)
{
    for(u32 i = 0; i < 9; ++i)
    {
        if(!AreEqual(a.m[i], b.m[i]))
            return false;
    }

    return true;
}

inline b32 operator!=(const mat3 &a, const mat3 &b)
{
    return !(a == b);
}

//
// NOTE: Functions
//

inline mat3 Transpose(const mat3 &m)
{
    mat3 result;

    result.xx = m.xx;
    result.xy = m.yx;
    result.xz = m.zx;
    result.yx = m.xy;
    result.yy = m.yy;
    result.yz = m.zy;
    result.zx = m.xz;
    result.zy = m.yz;
    result.zz = m.zz;
    
    return result;
}

inline void TransposeModify(mat3 &m)
{
    r32 temp = m.xy;
    m.xy = m.yx;
    m.yx = temp;

    temp = m.xz;
    m.xz = m.zx;
    m.zx = temp;

    temp = m.yz;
    m.yz = m.zy;
    m.zy = temp;
}

inline mat3 Adjoint(const mat3 &m)
{
    mat3 result;

    result.xx = (m.yy * m.zz - m.yz * m.zy);
    result.xy = (m.xy * m.zz - m.xz * m.zy);
    result.xz = (m.xy * m.yz - m.xz * m.yy);

    result.yx = (m.yx * m.zz - m.yz * m.zx);
    result.yy = (m.xx * m.zz - m.xz * m.zx);
    result.yz = (m.xx * m.yz - m.xz * m.yx);
    
    result.zx = (m.yx * m.zy - m.yy * m.zx);
    result.zy = (m.xx * m.zy - m.xy * m.zx);
    result.zz = (m.xx * m.yy - m.xy * m.yx);

    return result;
}

inline r32 Determinant(const mat3 &m)
{
    return (m.xx * (m.yy * m.zz - m.yz * m.zy)
          - m.xy * (m.yx * m.zz - m.yz * m.zx)
          + m.xz * (m.yx * m.zy - m.yy * m.zx));
}

inline mat3 Inverse(const mat3 &m)
{
    mat3 result;

    r32 det = Determinant(m);

    if(IsZero(det))
    {
        // NOTE: assert?
        result = MAT3_IDENTITY;
        return result;
    }

    r32 invdet = 1.0f / det;
    
    result = invdet * Adjoint(m);

    return result;
}

inline mat3 Hadamard(const mat3 &a, const mat3 &b)
{
    mat3 result;

    result.x = Hadamard(a.x, b.x);
    result.y = Hadamard(a.y, b.y);
    result.z = Hadamard(a.z, b.z);

    return result;
}

void SetRotation(mat3 &m, const r32 x, const r32 y, const r32 z)
{
    r32 sx, cx;
    SinCos(x, sx, cx);

    r32 sy, cy;
    SinCos(y, sy, cy);

    r32 sz, cz;
    SinCos(z, sz, cz);

    m.xx = cy * cz;
    m.xy = -(cy * sz);
    m.xz = sy;
    m.yx = (sx * sy * cz) + (cx * sz);
    m.yy = -(sx * sy * sz) + (cx * cz);
    m.yz = -(sx * cy);
    m.zx = -(cx * sy * cz) + (sx * sz);
    m.zy = (cx * sy * sz) + (sx * cz);
    m.zz = cx * cy;
}

inline void SetRotation(mat3 &m, const v3 &v)
{
    SetRotation(m, v.x, v.y, v.z);
}

void SetRotation(mat3 &m, v3 v, r32 a)
{
    r32 s, c, t;
    SinCos(a, s, c);
    t = 1.0f - c;

    Normalize(v);

    r32 tx = t * v.x, ty = t * v.y, tz = t * v.z,
        sx = s * v.x, sy = s * v.y, sz = s * v.z,
        txy = tx * v.y, tyz = tx * v.z, txz = tx * v.z;

    m.xx = tx * v.x + c;
    m.xy = txy - sz;
    m.xz = txz + sy;
    m.yx = txy + sz;
    m.yy = ty * v.y + c;
    m.yz = tyz - sx;
    m.zx = txz - sy;
    m.zy = tyz + sx;
    m.zz = tz * v.z + c;
}

void SetRotation(mat3 &m, quat &q)
{
    // NOTE: Assert unit quaternion

    r32 xs, ys, zs,
        wx, wy, wz,
        xx, xy, xz,
        yy, yz, zz;

    xs = q.x + q.x;
    ys = q.y + q.y;
    zs = q.z + q.z;
    wx = q.w * xs;
    wy = q.w * ys;
    wz = q.w * zs;
    xx = q.x * xs;
    xy = q.x * ys;
    xz = q.x * zs;
    yy = q.y * ys;
    yz = q.y * zs;
    zz = q.z * zs;

    m.xx = 1.0f - (yy + zz);
    m.xy = xy - wz;
    m.xz = xz + wy;

    m.yx = xy + wz;
    m.yy = 1.0f - (xx + zz);
    m.yz = yz - wx;

    m.zx = xz - wy;
    m.zy = yz + wx;
    m.zz = 1.0f - (xx + yy);
}

inline void SetRotationX(mat3 &m, const r32 a)
{
    r32 s, c;
    SinCos(a, s, c);

    m.x = {1.0f, 0.0f, 0.0f};
    m.y = {0.0f, c, -s};
    m.z = {0.0f, s, c};
}

inline void SetRotationY(mat3 &m, const r32 a)
{
    r32 s, c;
    SinCos(a, s, c);

    m.x = {c, 0.0f, s};
    m.y = {0.0f, 1.0f, 0.0f};
    m.z = {-s, 0.0f, c};
}

inline void SetRotationZ(mat3 &m, const r32 a)
{
    r32 s, c;
    SinCos(a, s, c);

    m.x = {c, -s, 0.0f};
    m.y = {s, c, 0.0f};
    m.z = {0.0f, 0.0f, 1.0f};
}

inline void SetScale(mat3 &m, const r32 x, const r32 y, const r32 z)
{
    m.x = {x, 0.0f, 0.0f};
    m.y = {0.0f, y, 0.0f};
    m.z = {0.0f, 0.0f, z};
}

inline void SetScale(mat3 &m, const v3 &v)
{
    SetScale(m, v.x, v.y, v.z);
}

// NOTE: See Mike Day, Extracting Euler Angles from a Rotation Matrix
// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2012/07/euler-angles1.pdf}
v3 GetEulerAngles(const mat3 &m)
{
    v3 result;

    result.x = atan2f(m.yz, m.zz);
    r32 c2 = AASqrt((m.xx * m.xx) + (m.xy * m.xy));
    result.y = atan2f(-m.xz, c2);
    r32 s, c;
    SinCos(result.x, s, c);
    result.z = atan2f((s * m.zx) - (c * m.yx), (c * m.yy) - (s * m.zy));

    return result;
}

void GetAxisAngle(const mat3 &m, v3 &axis, r32 &angle)
{
    r32 trace = m.xx + m.yy + m.zz;
    angle = acosf(0.5f * (trace - 1.0f));

    if(IsZero(angle))
    {
        // NOTE: Could be anything if angle is zero, so return the up/Y axis
        axis = VEC3_YAXIS;
    }
    else if(angle < PI - EPSILON)
    {
        axis = {m.zy - m.yz, m.xz - m.zx, m.yx - m.xy};
        Normalize(axis);
    }
    else
    {
        r32 sqrt, oneOverSqrt;

        if(m.xx > m.yy && m.xx > m.zz)
        {
            sqrt = AASqrt(m.xx - m.yy - m.zz + 1.0f);
            axis.x = 0.5f * sqrt;
            oneOverSqrt = 1.0f / sqrt;
            axis.y = m.xy * oneOverSqrt;
            axis.z = m.xz * oneOverSqrt;
        }
        else if(m.yy > m.zz)
        {
            sqrt = AASqrt(m.yy - m.xx - m.zz + 1.0f);
            axis.y = 0.5f * sqrt;
            oneOverSqrt = 1.0f / sqrt;
            axis.x = m.xy * oneOverSqrt;
            axis.z = m.yz * oneOverSqrt;
        }
        else
        {
            sqrt = AASqrt(m.zz - m.xx - m.yy + 1.0f);
            axis.z = 0.5f * sqrt;
            oneOverSqrt = 1.0f / sqrt;
            axis.x = m.xz * oneOverSqrt;
            axis.y = m.yz * oneOverSqrt;
        }
    }
}

quat GetQuaternion(const mat3 &m)
{
    quat result;
    r32 trace = 1.0f + m.xx + m.yy + m.zz;

    if (trace > EPSILON)
    {
        r32 s = AASqrt(trace) * 2.0f;
        result.w = s / 4.0f;
        result.x = (m.zy - m.yz) / s;
        result.y = (m.xz - m.zx) / s;
        result.z = (m.yx - m.xy) / s;
    }
    else if (m.xx > m.yy && m.xx > m.zz)
    {
        r32 s = AASqrt(1.0f + m.xx - m.yy - m.zz) * 2.0f;
        result.w = (m.zy - m.yz) / s;
        result.x = s / 4.0f;
        result.y = (m.yx + m.xy) / s;
        result.z = (m.xz + m.zx) / s;
    }
    else if (m.yy > m.zz)
    {
        r32 s = AASqrt(1.0f + m.yy - m.xx - m.zz) * 2.0f;
        result.w = (m.xz - m.zx) / s;
        result.x = (m.yx + m.xy) / s;
        result.y = s / 4.0f;
        result.z = (m.zy + m.yz) / s;
    }
    else
    {
        r32 s = AASqrt(1.0f + m.zz - m.xx - m.yy) * 2.0f;
        result.w = (m.yx - m.xy) / s;
        result.x = (m.xz + m.zx) / s;
        result.y = (m.zy + m.yz) / s;
        result.z = s / 4.0f;
    }

    return result;
}

#endif


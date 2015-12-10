#ifndef MAT4_H
#define MAT4_H

#include "aamath.h"
#include "quat.h"

union mat4
{
    struct
    {
        r32 xx, xy, xz, xw,
            yx, yy, yz, yw,
            zx, zy, zz, zw,
            tx, ty, tz, w;
    };
    struct
    {
        v4 x, y, z, t;
    };
    r32 m[16];
};

//
// NOTE: Static constants
//

static const mat4 MAT4_IDENTITY = {1.0f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f,
                                   0.0f, 0.0f, 0.0f, 1.0f};

//
// NOTE: Operators
//

inline mat4 operator-(const mat4 &a)
{
    mat4 result;

    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.t = -a.t;

    return result;
}

inline mat4 operator+(const mat4 &a, const r32 &b)
{
    mat4 result;

    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;
    result.t = a.t + b;

    return result;
}

inline mat4 operator+(const r32 &b, const mat4 &a)
{
    mat4 result;

    result.x = a.x + b;
    result.y = a.y + b;
    result.z = a.z + b;
    result.t = a.t + b;

    return result;
}

inline mat4 operator+(const mat4 &a, const mat4 &b)
{
    mat4 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.t = a.t + b.t;

    return result;
}

inline mat4 operator-(const mat4 &a, const r32 &b)
{
    mat4 result;

    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;
    result.t = a.t - b;

    return result;
}

inline mat4 operator-(const r32 &b, const mat4 &a)
{
    mat4 result;

    result.x = a.x - b;
    result.y = a.y - b;
    result.z = a.z - b;
    result.t = a.t - b;

    return result;
}

inline mat4 operator-(const mat4 &a, const mat4 &b)
{
    mat4 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.t = a.t - b.t;

    return result;
}

inline mat4 operator*(const mat4 &a, const r32 &b)
{
    mat4 result;

    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    result.t = a.t * b;

    return result;
}

inline mat4 operator*(const r32 &b, const mat4 &a)
{
    mat4 result;

    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    result.t = a.t * b;

    return result;
}

// NOTE: row vector * matrix
inline v4 operator*(const v4 &a, const mat4 &b)
{
    v4 result;

    result.x = a.x * b.xx + a.y * b.yx + a.z * b.zx + a.w * b.xw;
    result.y = a.x * b.xy + a.y * b.yy + a.z * b.zy + a.w * b.yw;
    result.z = a.x * b.xz + a.y * b.yz + a.z * b.zz + a.w * b.zw;
    result.w = a.x * b.tx + a.y * b.ty + a.z * b.tz + a.w * a.w;

    return result;
}

// NOTE: matrix * column vector
inline v4 operator*(const mat4 &a, const v4 &b)
{
    v4 result;

    result.x = a.xx * b.x + a.xy * b.y + a.xz * b.z + a.xw * b.w;
    result.y = a.yx * b.x + a.yy * b.y + a.yz * b.z + a.yw * b.w;
    result.z = a.zx * b.x + a.zy * b.y + a.zz * b.z + a.zw * b.w;
    result.w = a.tx * b.x + a.ty * b.y + a.tz * b.z + a.w * b.w;

    return result;
}

inline mat4 operator*(const mat4 &a, const mat4 &b)
{
    mat4 result;

    result.xx = a.xx * b.xx + a.xy * b.yx + a.xz * b.zx + a.xw * b.tx;
    result.xy = a.xx * b.xy + a.xy * b.yy + a.xz * b.zy + a.xw * b.ty;
    result.xz = a.xx * b.xz + a.xy * b.yz + a.xz * b.zz + a.xw * b.tz;
    result.xw = a.xx * b.xw + a.xy * b.yw + a.xz * b.zw + a.xw * b.w;

    result.yx = a.yx * b.xx + a.yy * b.yx + a.yz * b.zx + a.yw * b.tx;
    result.yy = a.yx * b.xy + a.yy * b.yy + a.yz * b.zy + a.yw * b.ty;
    result.yz = a.yx * b.xz + a.yy * b.yz + a.yz * b.zz + a.yw * b.tz;
    result.yw = a.yx * b.xw + a.yy * b.yw + a.yz * b.zw + a.yw * b.w;

    result.zx = a.zx * b.xx + a.zy * b.yx + a.zz * b.zx + a.zw * b.tx;
    result.zy = a.zx * b.xy + a.zy * b.yy + a.zz * b.zy + a.zw * b.ty;
    result.zz = a.zx * b.xz + a.zy * b.yz + a.zz * b.zz + a.zw * b.tz;
    result.zw = a.zx * b.xw + a.zy * b.yw + a.zz * b.zw + a.zw * b.w;

    result.tx = a.tx * b.xx + a.ty * b.yx + a.tz * b.zx + a.w * b.tx;
    result.ty = a.tx * b.xy + a.ty * b.yy + a.tz * b.zy + a.w * b.ty;
    result.tz = a.tx * b.xz + a.ty * b.yz + a.tz * b.zz + a.w * b.tz;
    result.w  = a.tx * b.xw + a.ty * b.yw + a.tz * b.zw + a.w * b.w;

    return result;
}

inline mat4 operator/(const mat4 &a, const r32 &b)
{
    mat4 result;

    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    result.t = a.t / b;

    return result;
}

inline mat4 operator/(const r32 &b, const mat4 &a)
{
    mat4 result;

    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    result.t = a.t / b;

    return result;
}

inline mat4 &operator+=(mat4 &a, const r32 &b)
{
    a.x = a.x + b;
    a.y = a.y + b;
    a.z = a.z + b;
    a.t = a.t + b;

    return a;
}

inline mat4 &operator+=(mat4 &a, const mat4 &b)
{
    a.x = a.x + b.x;
    a.y = a.y + b.y;
    a.z = a.z + b.z;
    a.t = a.t + b.t;

    return a;
}

inline mat4 &operator-=(mat4 &a, const r32 &b)
{
    a.x -= b;
    a.y -= b;
    a.z -= b;
    a.t -= b;

    return a;
}

inline mat4 &operator-=(mat4 &a, const mat4 &b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.t -= b.t;

    return a;
}

inline mat4 &operator*=(mat4 &a, const r32 &b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.t *= b;

    return a;
}

// NOTE: row vector *= matrix
inline v4 &operator*=(v4 &a, const mat4 &b)
{
    a = a * b;

    return a;
}

inline mat4 &operator*=(mat4 &a, const mat4 &b)
{
    a = a * b;

    return a;
}

inline mat4 &operator/=(mat4 &a, const r32 &b)
{
    a = a / b;

    return a;
}

inline b32 operator==(const mat4 &a, const mat4 &b)
{
    for(u32 i = 0; i < 16; ++i)
    {
        if(!AreEqual(a.m[i], b.m[i]))
            return false;
    }

    return true;
}

inline b32 operator!=(const mat4 &a, const mat4 &b)
{
    return !(a == b);
}

//
// NOTE: Functions
//

inline mat4 Transpose(const mat4 &m)
{
    mat4 result;

    result.xx = m.xx;
    result.xy = m.yx;
    result.xz = m.zx;
    result.xw = m.tx;
    result.yx = m.xy;
    result.yy = m.yy;
    result.yz = m.zy;
    result.yw = m.ty;
    result.zx = m.xz;
    result.zy = m.yz;
    result.zz = m.zz;
    result.zw = m.tz;
    result.tx = m.xw;
    result.ty = m.yw;
    result.tz = m.zw;
    result.w = m.w;
    
    return result;
}

inline void TransposeModify(mat4 &m)
{
    r32 temp = m.xy;
    m.xy = m.yx;
    m.yx = temp;

    temp = m.xz;
    m.xz = m.zx;
    m.zx = temp;

    temp = m.xw;
    m.xw = m.tx;
    m.tx = temp;

    temp = m.yz;
    m.yz = m.zy;
    m.zy = temp;

    temp = m.yw;
    m.yw = m.ty;
    m.ty = temp;

    temp = m.zw;
    m.zw = m.tz;
    m.tz = temp;
}

inline mat4 Adjoint(const mat4 &m)
{
    mat4 result;

    result.xx = (m.yy * m.zz - m.yz * m.zy);
    result.xy = (m.xy * m.zz - m.xz * m.zy);
    result.xz = (m.xy * m.yz - m.xz * m.yy);

    result.yx = (m.yx * m.zz - m.yz * m.zx);
    result.yy = (m.xx * m.zz - m.xz * m.zx);
    result.yz = (m.xx * m.yz - m.xz * m.yx);
    
    result.zx = (m.yx * m.zy - m.yy * m.zx);
    result.zy = (m.xx * m.zy - m.xy * m.zx);
    result.zz = (m.xx * m.yy - m.xy * m.yx);

    result.xw = m.xw;
    result.xy = m.xy;
    result.xz = m.xz;

    result.t = m.t;

    return result;
}

inline r32 Determinant(const mat4 &m)
{
    return (m.xx * (m.yy * m.zz - m.yz * m.zy)
          - m.xy * (m.yx * m.zz - m.yz * m.zx)
          + m.xz * (m.yx * m.zy - m.yy * m.zx));
}

// NOTE: Get matrix's upper 3x3 matrix (rotation + scaling)
inline mat3 Upper3x3(const mat4 &m)
{
    mat3 result;

    result.x = m.x.xyz;
    result.y = m.y.xyz;
    result.z = m.z.xyz;

    return result;
}

inline mat4 Inverse(const mat4 &m)
{
    mat4 result;

    r32 det = Determinant(m);

    if(IsZero(det))
    {
        // NOTE: assert?
        result = MAT4_IDENTITY;
        return result;
    }

    r32 invdet = 1.0f / det;
    
    result = invdet * Adjoint(m);

    // NOTE: should probably do this directly
    result.t.xyz *= -Upper3x3(m);

    return result;
}

inline mat4 Hadamard(const mat4 &a, const mat4 &b)
{
    mat4 result;

    result.x = Hadamard(a.x, b.x);
    result.y = Hadamard(a.y, b.y);
    result.z = Hadamard(a.z, b.z);
    result.t = Hadamard(a.t, b.t);

    return result;
}

// NOTE: Rotation by Euler (x, y, z)
void SetRotation(mat4 &m, const r32 x, const r32 y, const r32 z)
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

// NOTE: Proxy for above
inline void SetRotation(mat4 &m, const v3 &v)
{
    SetRotation(m, v.x, v.y, v.z);
}

// NOTE: Axis angle rotation
void SetRotation(mat4 &m, v3 v, r32 a)
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

void SetRotation(mat4 &m, quat &q)
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

inline void SetRotationX(mat4 &m, const r32 a)
{
    r32 s, c;
    SinCos(a, s, c);

    m.x = {1.0f, 0.0f, 0.0f};
    m.y = {0.0f, c, -s};
    m.z = {0.0f, s, c};
}

inline void SetRotationY(mat4 &m, const r32 a)
{
    r32 s, c;
    SinCos(a, s, c);

    m.x = {c, 0.0f, s};
    m.y = {0.0f, 1.0f, 0.0f};
    m.z = {-s, 0.0f, c};
}

inline void SetRotationZ(mat4 &m, const r32 a)
{
    r32 s, c;
    SinCos(a, s, c);

    m.x = {c, -s, 0.0f};
    m.y = {s, c, 0.0f};
    m.z = {0.0f, 0.0f, 1.0f};
}

inline void SetScale(mat4 &m, const r32 x, const r32 y, const r32 z)
{
    m.x = {x, 0.0f, 0.0f};
    m.y = {0.0f, y, 0.0f};
    m.z = {0.0f, 0.0f, z};
}

inline void SetScale(mat4 &m, const v3 &v)
{
    SetScale(m, v.x, v.y, v.z);
}

// NOTE: See Mike Day, Extracting Euler Angles from a Rotation Matrix
// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2012/07/euler-angles1.pdf}
v3 GetEulerAngles(const mat4 &m)
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

void GetAxisAngle(const mat4 &m, v3 &axis, r32 &angle)
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

quat GetQuaternion(const mat4 &m)
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


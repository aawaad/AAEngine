#include <stdio.h>
#include <stdint.h>
#include "aamath.h"

/*
    TODO: x = done, \ = partial, ? = consider

    Vectors:
    x   v2, v3, v4    (float)  
    x   v2s, v3s, v4s (int)
    x   v2u, v3u, v4u (uint)

    x   operators

    x   cross, perpendicular (2D)
    x   distance
    x   distance squared
    x   dot
    x   hadamard product
    x   length
    x   length squared
    x   normalize
    x   reflect
    x   refract
    x   scalar triple product
    x   vector triple product

    x   IsZero
    x   IsUnit

    x   statics (x, y, z axes; w; origin (0, 0, 0, 0))

    Matrices:
    x   mat3, mat4
   ?    mat3i, mat4i

    x   operators

    x   adjoint
    x   determinant
    x   identity
    x   inverse
    x   hadamard product
    x   transpose
        
        set:
   ?    translation     NOTE: this probably isn't necessary with unions (e.g. mat.t.xyz = vec3 / mat.t = vec4)
    x   rotation (quaternion)
    x   rotation (pitch, yaw, roll)
    x   rotation (axis, angle)
    x   rotation (x, y, z separate)
    x   scale

        get:
    x   to euler angles
    x   to axis angle
    x   to quaternion

    Quaternions:
    x   magnitude
    x   norm
    x   normalize
    x   conjugate
    x   inverse

    x   operators

    x   dot
    x   rotate
    x   lerp
    x   slerp
    x   approx slerp

    x   zero
    x   identity

    Lines:
        line2, line3

        operators

        transform
        distance
        closest points

    Rays:
        ray2, ray3

        operators
        
        transform
        ditance
        closest points

    Triangles:
        point in triangle
        barycentric coords
        triangle-triangle intersection
        triangle-ray intersection

    Planes:
        transform
        distance
        closest point
        plane test

    Common:
    x   constants (pi, tau/2pi, halfpi, epsilon etc) 
        
        abs
        ceil
        floor
    x   areequal
    x   iszero
        isinf
        isnan
        min
        max
        round
        lerp

    Collision:
        AABB
        OBB
        Bounding spheres
        Capsules

    SIMD optimisations?
    Self-implemented standard library functions (cos, sin etc)?

*/

int main(int argv, char **argc)
{
    v2 dim2 = {};
    dim2.x = 0.5f;
    dim2.y = 0.5f;

    v3 dim3 = {};
    dim3.x = 0.5f;
    dim3.y = 0.5f;
    dim3.z = 0.5f;
    
    for (u32 i = 0; i < 3; ++i)
    {
        dim3.E[i] = (r32)i;
    }

    v4 dim4 = {};
    dim4.xyz = dim3;
    dim4.a = 0.0f;

    v3 x = {1.0f, 0.0f, 0.0f};
    v3 y = {1.0f, 1.0f, 0.0f};
    Normalize(y);

    printf("dot: %.3f\n", Dot(x, y));
    printf("dot: %.3f\n", Dot(VEC3_XAXIS, y));

    v2 dir2 = {1, 0};
    v2 dir2perp = Perpendicular(dir2);

    printf("perp: %3.f, %3.f\n", dir2perp.x, dir2perp.y);

    y = {0.0f, 1.0f, 0.0f};
    v3 cross = Cross(x, y);

    printf("cross: <%.3f, %.3f, %.3f>\n", cross.x, cross.y, cross.z);

    y = y + x;
    y = y - x;
    y = -y;
    y = 2.0f * y;
    y *= 2.0f;
    y += x;
    y == x;

    printf("sqrt 9: %f\n", AASqrt(9.f));
    printf("inv sqrt 9: %f\n", InvSqrt(9.f));

    return 0;
}


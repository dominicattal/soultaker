#ifndef VEC_H
#define VEC_H

#include "type.h"

typedef struct {
    f32 x;
    union {
        f32 y, z;
    };
} vec2;

typedef struct {
    f32 x, y, z;
} vec3;

typedef struct {
    vec3 normal;
    f32 distance;
} Plane;

typedef struct {
    vec3 origin;
    vec3 direction;
} Ray;

typedef struct {
    vec3 a, b;
} Segment;

typedef struct {
    vec3 a, b, c;
} Triangle;

typedef struct {
    vec3 origin;
    f32 radius;
} Sphere;

typedef struct {
    vec3 origin;
    vec3 offset;
} AABB;

typedef struct {
    vec3 origin;
    vec3 offset;
    vec3 orientation;
} OBB;

typedef struct {
    vec3 points[8];
    Plane planes[6];
} Hexahedron;

vec2 vec2_create(f32 x, f32 y);
vec2 vec2_add(vec2 v1, vec2 v2);
vec2 vec2_sub(vec2 v1, vec2 v2);
vec2 vec2_normalize(vec2 vec);
vec2 vec2_scale(vec2 vec, f32 scale);
vec2 vec2_direction(f32 rad);
vec2 vec2_rotate(vec2 vec, f32 rad);
f32  vec2_radians(vec2 vec);
f32  vec2_mag(vec2 vec);
f32  vec2_distance(vec2 v1, vec2 v2);
f32  vec2_dot(vec2 v1, vec2 v2);
bool vec2_equal(vec2 v1, vec2 v2);

vec3 vec3_create(f32 x, f32 y, f32 z);
vec3 vec3_add(vec3 v1, vec3 v2);
vec3 vec3_sub(vec3 v1, vec3 v2);
vec3 vec3_normalize(vec3 vec);
vec3 vec3_scale(vec3 vec, f32 scale);
vec3 vec3_cross(vec3 v1, vec3 v2);
f32  vec3_dot(vec3 v1, vec3 v2);
f32  vec3_mag(vec3 vec);
void vec3_print(vec3 vec);

Ray ray_create(vec3 origin, vec3 direction);
Segment segment_create(vec3 a, vec3 b);
Triangle triangle_create(vec3 a, vec3 b, vec3 c);
AABB aabb_create(vec3 origin, vec3 offset);
OBB obb_create(vec3 origin, vec3 offset, vec3 orientation);

f32  intersect_ray_triangle(Ray ray, Triangle triangle);
bool intersect_segment_triangle(Segment segment, Triangle triangle);
bool intersect_triangle_triangle(Triangle triangle1, Triangle triangle2);
bool intersect_aabb_hexahedron(AABB aabb, Hexahedron hexahedron);

#endif

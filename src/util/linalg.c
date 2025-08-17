#include "linalg.h"
#include <math.h>
#include <stdio.h>

vec2 vec2_create(f32 x, f32 y)
{
    return (vec2) { .x = x, .y = y };
}

vec2 vec2_add(vec2 v1, vec2 v2)
{
    return (vec2) { .x = v1.x + v2.x, .y = v1.y + v2.y };
}

vec2 vec2_sub(vec2 v1, vec2 v2)
{
    return (vec2) { .x = v1.x - v2.x, .y = v1.y - v2.y };
}

vec2 vec2_normalize(vec2 vec)
{
    f32 mag = vec2_mag(vec);
    if (mag == 0)
        return vec;
    return (vec2) { .x = vec.x / mag, .y = vec.y / mag };

}

vec2 vec2_scale(vec2 vec, f32 scale)
{
    return (vec2) { .x = vec.x * scale, .y = vec.y * scale };
}

vec2 vec2_direction(f32 rad)
{
    return (vec2) { .x = cos(rad), .y = sin(rad) };
}

vec2 vec2_rotate(vec2 vec, f32 rad)
{
    return (vec2) { 
        .x = vec.x * cos(rad) - vec.y * sin(rad),
        .y = vec.x * sin(rad) + vec.y * cos(rad)
    };
}

f32 vec2_radians(vec2 vec)
{
    return atan(vec.y / vec.x) + ((vec.x < 0) ? PI : 0);
}

f32 vec2_mag(vec2 vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}

f32 vec2_distance(vec2 v1, vec2 v2)
{
    f32 dx = v2.x - v1.x;
    f32 dy = v2.y - v1.y;
    return sqrt(dx * dx + dy * dy);
}

f32 vec2_dot(vec2 v1, vec2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

bool vec2_equal(vec2 v1, vec2 v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}

vec3 vec3_create(f32 x, f32 y, f32 z)
{
    return (vec3) { x, y, z };
}

vec3 vec3_add(vec3 v1, vec3 v2)
{
    return (vec3) { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

vec3 vec3_sub(vec3 v1, vec3 v2)
{
    return (vec3) { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

vec3 vec3_normalize(vec3 vec)
{
    f32 mag = vec3_mag(vec);
    if (mag == 0)
        return vec;
    return (vec3) { vec.x / mag, vec.y / mag, vec.z / mag };
}

vec3 vec3_scale(vec3 vec, f32 scale)
{
    return (vec3) { vec.x * scale, vec.y * scale, vec.z * scale };
}

vec3 vec3_cross(vec3 v1, vec3 v2)
{
    return (vec3) {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}

f32 vec3_dot(vec3 v1, vec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

f32 vec3_mag(vec3 vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

void vec3_print(vec3 vec)
{
    printf("(%.5f,%.5f,%.5f)\n", vec.x, vec.y, vec.z);
}

Ray ray_create(vec3 origin, vec3 direction)
{
    return (Ray) { origin, direction };
}

Segment segment_create(vec3 a, vec3 b)
{
    return (Segment) { a, b };
}

Triangle triangle_create(vec3 a, vec3 b, vec3 c)
{
    return (Triangle) { a, b, c };
}

AABB aabb_create(vec3 origin, vec3 offset)
{
    return (AABB) { origin, offset };
}

OBB obb_create(vec3 origin, vec3 offset, vec3 orientation)
{
    return (OBB) { origin, offset, orientation };
}

f32 intersect_ray_triangle(Ray ray, Triangle triangle)
{
    // Möller–Trumbore
    vec3 edge1 = vec3_sub(triangle.b, triangle.a);
    vec3 edge2 = vec3_sub(triangle.c, triangle.a);
    vec3 ray_cross_e2 = vec3_cross(ray.direction, edge2);
    f32 det = vec3_dot(edge1, ray_cross_e2);

    if (det > -EPSILON && det < EPSILON)
        return -INF;

    f32 inv_det = 1 / det;
    vec3 s = vec3_sub(ray.origin, triangle.a);
    f32 u = inv_det * vec3_dot(s, ray_cross_e2);

    if ((u < 0 && fabs(u) > EPSILON) || (u > 1 && fabs(u - 1) > EPSILON))
        return -INF;

    vec3 s_cross_e1 = vec3_cross(s, edge1);
    f32 v = inv_det * vec3_dot(ray.direction, s_cross_e1);

    if ((v < 0 && fabs(v) > EPSILON) || (u + v > 1 && fabs(u + v - 1) > EPSILON))
        return -INF;
    
    return inv_det * vec3_dot(edge2, s_cross_e1);
}

bool intersect_segment_triangle(Segment segment, Triangle triangle)
{
    // modified Möller–Trumbore
    vec3 ray_origin = segment.a;
    vec3 ray_direction = vec3_sub(segment.b, segment.a);
    vec3 edge1 = vec3_sub(triangle.b, triangle.a);
    vec3 edge2 = vec3_sub(triangle.c, triangle.a);
    vec3 ray_cross_e2 = vec3_cross(ray_direction, edge2);
    f32 det = vec3_dot(edge1, ray_cross_e2);

    if (det > -EPSILON && det < EPSILON)
        return false;

    f32 inv_det = 1 / det;
    vec3 s = vec3_sub(ray_origin, triangle.a);
    f32 u = inv_det * vec3_dot(s, ray_cross_e2);

    if ((u < 0 && fabs(u) > EPSILON) || (u > 1 && fabs(u - 1) > EPSILON))
        return false;

    vec3 s_cross_e1 = vec3_cross(s, edge1);
    f32 v = inv_det * vec3_dot(ray_direction, s_cross_e1);

    if ((v < 0 && fabs(v) > EPSILON) || (u + v > 1 && fabs(u + v - 1) > EPSILON))
        return false;
    
    f32 t = inv_det * vec3_dot(edge2, s_cross_e1);
    return t > -EPSILON && t < 1 + EPSILON;
}

bool intersect_triangle_triangle(Triangle triangle1, Triangle triangle2)
{
    return !(!intersect_segment_triangle(segment_create(triangle1.a, triangle1.b), triangle2)
          && !intersect_segment_triangle(segment_create(triangle1.b, triangle1.c), triangle2)
          && !intersect_segment_triangle(segment_create(triangle1.c, triangle1.a), triangle2)
          && !intersect_segment_triangle(segment_create(triangle2.a, triangle2.b), triangle1)
          && !intersect_segment_triangle(segment_create(triangle2.b, triangle2.c), triangle1)
          && !intersect_segment_triangle(segment_create(triangle2.c, triangle2.a), triangle1));
}

bool intersect_aabb_hexahedron(AABB aabb, Hexahedron hexahedron)
{
    vec3 points[8];
    vec3 normal;
    f32 distance;
    i32 i, j;
    f32 x, y, z, dx, dy, dz;
    x = aabb.origin.x; y = aabb.origin.y; z = aabb.origin.z;
    dx = aabb.offset.x; dy = aabb.offset.y; dz = aabb.offset.z;
    points[0] = vec3_create(x     , y     , z     );
    points[1] = vec3_create(x + dx, y     , z     );
    points[2] = vec3_create(x + dx, y + dy, z     );
    points[3] = vec3_create(x     , y + dy, z     );
    points[4] = vec3_create(x     , y     , z + dz);
    points[5] = vec3_create(x + dx, y     , z + dz);
    points[6] = vec3_create(x + dx, y + dy, z + dz);
    points[7] = vec3_create(x     , y + dy, z + dz);
    
    // test if a point is in frustrum
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 6; j++) {
            normal = hexahedron.planes[j].normal;
            distance = hexahedron.planes[j].distance;
            if ((vec3_dot(normal, points[i]) + distance) < 0)
                goto next_point1;
        }
        return true;
        next_point1:
    }

    static u8 triangles[] = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        0, 1, 5, 0, 4, 5,
        2, 3, 6, 2, 6, 7,
        1, 2, 5, 1, 5, 6,
        0, 3, 4, 0, 4, 7
    };

    Triangle triangle1, triangle2;
    for (i = 0; i < 12; i++) {
        triangle1.a = hexahedron.points[triangles[3*i]];
        triangle1.b = hexahedron.points[triangles[3*i+1]];
        triangle1.c = hexahedron.points[triangles[3*i+2]];
        for (j = 0; j < 12; j++) {
            triangle2.a = points[triangles[3*j]];
            triangle2.b = points[triangles[3*j+1]];
            triangle2.c = points[triangles[3*j+2]];
            if (intersect_triangle_triangle(triangle1, triangle2))
                return true;
        }
    }

    return false;
}

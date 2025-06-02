#ifndef TYPE_H
#define TYPE_H

#include <stdint.h>
#include <stdbool.h>

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

#define PI      3.141592653589
#define HALFPI  1.57079632679
#define EPSILON 1e-5
#define INF     1e10

#define UP      0
#define RIGHT   1
#define DOWN    2
#define LEFT    3

#define NEGX    0
#define POSX    1
#define NEGZ    2
#define POSZ    3
#define NEGY    4
#define POSY    5

#endif

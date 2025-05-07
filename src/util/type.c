#include "type.h"
#include <math.h>
#include <stdio.h>

i32 get_direction(f32 rad)
{
    rad = fmod(rad, 2*PI);
    if (rad < 0) rad += 2*PI;
    if (rad > 7 * PI / 4 || rad < PI / 4)
        return POSX;
    if (rad < 3 * PI / 4)
        return POSZ;
    if (rad < 5 * PI / 4)
        return NEGX;
    return NEGZ;
}

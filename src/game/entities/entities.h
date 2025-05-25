#ifndef ENTITIES_H
#define ENTITIES_H

#define TT_INSERT(DIR, STATE, FRAME, TEX) \
    texture_table[ DIR ][ STATE ][ FRAME ] = TEX

// Knight
typedef enum {
    KNIGHT_STATE_IDLE,
    KNIGHT_STATE_WALKING,
    KNIGHT_STATE_SHOOTING,
    NUM_KNIGHT_STATES
} KnightStates;

#endif

#include "character.h"

Character char_map[128];

#define _CHAR_INIT(_c, _tex, _w, _h, _bx, _by, _adv) \
    char_map[_c].tex = _tex; \
    char_map[_c].size.x = _w; \
    char_map[_c].size.y = _h; \
    char_map[_c].bearing.x = _bx; \
    char_map[_c].bearing.y = _by; \
    char_map[_c].advance = _adv;

void char_map_init(void)
{
    for (i32 i = 0; i < 128; i++) {
        _CHAR_INIT(i, EMPTY_TEX, 5, 7, 0, 0, 5);
    }
    _CHAR_INIT(' ', NO_TEX,      5, 7, 0, 0, 5)
    _CHAR_INIT('A', UPPER_A_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('B', UPPER_B_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('C', UPPER_C_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('D', UPPER_D_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('E', UPPER_E_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('F', UPPER_F_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('G', UPPER_G_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('H', UPPER_H_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('I', UPPER_I_TEX, 3, 7, 0, 0, 3)
    _CHAR_INIT('J', UPPER_J_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('K', UPPER_K_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('L', UPPER_L_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('M', UPPER_M_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('N', UPPER_N_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('O', UPPER_O_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('P', UPPER_P_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('Q', UPPER_Q_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('R', UPPER_R_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('S', UPPER_S_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('T', UPPER_T_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('U', UPPER_U_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('V', UPPER_V_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('W', UPPER_W_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('X', UPPER_X_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('Y', UPPER_Y_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('Z', UPPER_Z_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('a', LOWER_A_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('b', LOWER_B_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('c', LOWER_C_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('d', LOWER_D_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('e', LOWER_E_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('f', LOWER_F_TEX, 4, 7, 0, 0, 4)
    _CHAR_INIT('g', LOWER_G_TEX, 5, 6, 0, -1, 5)
    _CHAR_INIT('h', LOWER_H_TEX, 5, 7, 0, 0, 5)
    _CHAR_INIT('i', LOWER_I_TEX, 1, 7, 1, 0, 3)
    _CHAR_INIT('j', LOWER_J_TEX, 5, 8, 0, -1, 5)
    _CHAR_INIT('k', LOWER_K_TEX, 4, 7, 0, 0, 4)
    _CHAR_INIT('l', LOWER_L_TEX, 2, 7, 0, 0, 2)
    _CHAR_INIT('m', LOWER_M_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('n', LOWER_N_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('o', LOWER_O_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('p', LOWER_P_TEX, 5, 6, 0, -1, 5)
    _CHAR_INIT('q', LOWER_Q_TEX, 5, 6, 0, -1, 5)
    _CHAR_INIT('r', LOWER_R_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('s', LOWER_S_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('t', LOWER_T_TEX, 3, 7, 0, 0, 3)
    _CHAR_INIT('u', LOWER_U_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('v', LOWER_V_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('w', LOWER_W_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('x', LOWER_X_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('y', LOWER_Y_TEX, 5, 6, 0, -1, 5)
    _CHAR_INIT('z', LOWER_Z_TEX, 5, 5, 0, 0, 5)
    _CHAR_INIT('0', NUM_0_TEX,   5, 7, 0, 0, 5)
    _CHAR_INIT('1', NUM_1_TEX,   3, 7, 0, 0, 3)
    _CHAR_INIT('2', NUM_2_TEX,   5, 7, 0, 0, 5)
    _CHAR_INIT('3', NUM_3_TEX,   5, 7, 0, 0, 5)
    _CHAR_INIT('4', NUM_4_TEX,   5, 7, 0, 0, 5)
    _CHAR_INIT('5', NUM_5_TEX,   5, 7, 0, 0, 5)
    _CHAR_INIT('6', NUM_6_TEX,   5, 7, 0, 0, 5)
    _CHAR_INIT('7', NUM_7_TEX,   5, 7, 0, 0, 5)
    _CHAR_INIT('8', NUM_8_TEX,   5, 7, 0, 0, 5)
    _CHAR_INIT('9', NUM_9_TEX,   5, 7, 0, 0, 5)
    _CHAR_INIT('/', FSLASH_TEX , 3, 5, 0, 0, 4)
}
#include "speed.h"

#include "app_config.h"

int32_t speed_update(int32_t k)
{
    // TODO: k = 2k (mod N)
    //  - with N = SPEED_N = 3, k alternates 2 -> 1 -> 2 -> ... (2*2 mod 3 = 1, 2*1 mod 3 = 2):
    //    first press halves the step, second press restores it
    //  - careful: with a power-of-two N (e.g. 4) the sequence is 1 -> 2 -> 0 -> 0,
    //    and k = 0 means the servo never moves again
    return k;
}

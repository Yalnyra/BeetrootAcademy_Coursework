/*
 * Step-size rule of mini-project 3 (assignment logic, not a reusable driver)
 */
#pragma once

#include <stdint.h>

/* Update speed: k = 2k (mod N), N - maximum speed
 * Returns the new k. */
int32_t speed_update(int32_t k);

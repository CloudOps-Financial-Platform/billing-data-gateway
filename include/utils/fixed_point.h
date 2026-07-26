#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <stdint.h>
#include "../ifm_spec.h"

/*
 * Converts string decimal representation (e.g., "12.345678") into
 * micro-units (int64_t) using fixed-point arithmetic (1 USD = 1,000,000 micros).
 * Eliminates IEEE 754 floating-point rounding errors.
 */
int64_t parse_cost_micros(const str_slice_t *slice);

#endif /* FIXED_POINT_H */
#include "../../include/utils/fixed_point.h"

int64_t parse_cost_micros(const str_slice_t *slice)
{
    if (!slice || !slice->ptr || slice->len == 0)
        return 0;

    int64_t dollars = 0;
    int64_t micros = 0;
    size_t i = 0;

    /* Parse integer component */
    while (i < slice->len && slice->ptr[i] >= '0' && slice->ptr[i] <= '9')
    {
        dollars = dollars * 10 + (slice->ptr[i] - '0');
        i++;
    }

    /* Parse fractional component up to 6 decimal places (micro-units) */
    if (i < slice->len && slice->ptr[i] == '.')
    {
        i++;
        int64_t multiplier = 100000; /* First decimal place = 100,000 micros */
        while (i < slice->len && slice->ptr[i] >= '0' && slice->ptr[i] <= '9' && multiplier > 0)
        {
            micros += (slice->ptr[i] - '0') * multiplier;
            multiplier /= 10;
            i++;
        }
    }

    return (dollars * 1000000LL) + micros;
}
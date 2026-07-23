#include "../include/csv_parser.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

const char *ifm_status_to_string(ifm_status_e status)
{
    switch (status)
    {
    case IFM_OK:
        return "SUCCESS";
    case IFM_ERR_EMPTY_FILE:
        return "ERR: File is completely empty";
    case IFM_ERR_NULL_POINTER:
        return "ERR: Null pointer provided";
    case IFM_ERR_INSUFFICIENT_FIELDS:
        return "ERR: Missing expected CSV columns";
    case IFM_ERR_UNSUPPORTED_PROVIDER:
        return "ERR: Unknown/unsupported cloud vendor";
    case IFM_ERR_INVALID_NUMERIC_COST:
        return "ERR: Malformed floating-point cost value";
    case IFM_ERR_INVALID_TIMESTAMP:
        return "ERR: Malformed integer timestamp value";
    case IFM_ERR_TRUNCATED_ROW:
        return "ERR: Truncated CSV row";
    default:
        return "ERR: Unknown internal error";
    }
}

size_t csv_tokenize_line(const char *line_start, const char *line_end, str_slice_t *fields_out, size_t max_fields)
{
    if (!line_start || !line_end || !fields_out || line_start >= line_end)
        return 0;

    size_t count = 0;
    const char *curr = line_start;
    const char *field_start = curr;

    while (curr < line_end && count < max_fields)
    {
        if (*curr == ',' || *curr == '\r' || *curr == '\n')
        {
            fields_out[count].ptr = field_start;
            fields_out[count].len = (size_t)(curr - field_start);
            count++;
            field_start = curr + 1;
        }
        curr++;
    }

    if (curr > field_start && count < max_fields)
    {
        fields_out[count].ptr = field_start;
        fields_out[count].len = (size_t)(curr - field_start);
        count++;
    }

    return count;
}

bool safe_slice_to_double(str_slice_t slice, double *out_val)
{
    if (!out_val || slice.len == 0)
        return false;

    char buf[64];
    size_t copy_len = slice.len < 63 ? slice.len : 63;
    memcpy(buf, slice.ptr, copy_len);
    buf[copy_len] = '\0';

    char *endptr;
    errno = 0;
    double val = strtod(buf, &endptr);

    if (endptr == buf || errno == ERANGE)
    {
        return false; // Conversion error or overflow
    }

    *out_val = val;
    return true;
}

bool safe_slice_to_uint64(str_slice_t slice, uint64_t *out_val)
{
    if (!out_val || slice.len == 0)
        return false;

    char buf[64];
    size_t copy_len = slice.len < 63 ? slice.len : 63;
    memcpy(buf, slice.ptr, copy_len);
    buf[copy_len] = '\0';

    char *endptr;
    errno = 0;
    unsigned long long val = strtoull(buf, &endptr, 10);

    if (endptr == buf || errno == ERANGE)
    {
        return false;
    }

    *out_val = (uint64_t)val;
    return true;
}

void slice_to_cstring(str_slice_t slice, char *dest, size_t dest_max)
{
    if (!dest || dest_max == 0)
        return;
    size_t copy_len = slice.len < (dest_max - 1) ? slice.len : (dest_max - 1);
    memcpy(dest, slice.ptr, copy_len);
    dest[copy_len] = '\0';
}
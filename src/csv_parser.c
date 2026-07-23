#include "../include/csv_parser.h"
#include <stdlib.h>
#include <string.h>

size_t csv_tokenize_line(const char* line_start, const char* line_end, str_slice_t* fields_out, size_t max_fields) {
    if (!line_start || !line_end || !fields_out) return 0;

    size_t count = 0;
    const char* curr = line_start;
    const char* field_start = curr;

    while (curr < line_end && count < max_fields) {
        if (*curr == ',' || *curr == '\r' || *curr == '\n') {
            fields_out[count].ptr = field_start;
            fields_out[count].len = (size_t)(curr - field_start);
            count++;
            field_start = curr + 1;
        }
        curr++;
    }

    if (curr > field_start && count < max_fields) {
        fields_out[count].ptr = field_start;
        fields_out[count].len = (size_t)(curr - field_start);
        count++;
    }

    return count;
}

double slice_to_double(str_slice_t slice) {
    char buf[64];
    size_t copy_len = slice.len < 63 ? slice.len : 63;
    memcpy(buf, slice.ptr, copy_len);
    buf[copy_len] = '\0';
    return atof(buf);
}

uint64_t slice_to_uint64(str_slice_t slice) {
    char buf[64];
    size_t copy_len = slice.len < 63 ? slice.len : 63;
    memcpy(buf, slice.ptr, copy_len);
    buf[copy_len] = '\0';
    return (uint64_t)atoll(buf);
}

void slice_to_cstring(str_slice_t slice, char* dest, size_t dest_max) {
    if (dest_max == 0) return;
    size_t copy_len = slice.len < (dest_max - 1) ? slice.len : (dest_max - 1);
    memcpy(dest, slice.ptr, copy_len);
    dest[copy_len] = '\0';
}
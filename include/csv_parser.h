#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include "../include/ifm_core.h"

#define MAX_FIELDS_PER_ROW 32

size_t csv_tokenize_line(const char* line_start, const char* line_end, str_slice_t* fields_out, size_t max_fields);
double slice_to_double(str_slice_t slice);
uint64_t slice_to_uint64(str_slice_t slice);
void slice_to_cstring(str_slice_t slice, char* dest, size_t dest_max);

#endif // CSV_PARSER_H
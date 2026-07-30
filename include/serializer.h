#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdio.h>
#include <stdbool.h>
#include "ifm_spec.h"

/**
 * @brief Streams an array of normalized financial model records into a file handle as JSON.
 * @param stream Target stream handle (e.g., stdout or an open disk file descriptor).
 * @param records Collection array tracking internal binary records.
 * @param count Quantitative size of the record array.
 * @return Returns true upon successful stream write execution, false on validation failure.
 */
bool serializer_write_json(FILE *stream, const ifm_record_t *records, size_t count);

#endif
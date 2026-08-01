#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdio.h>
#include <stdbool.h>
#include "ifm_spec.h"

/**
 * @brief Streams an array of normalized financial model records into a file handle as JSON.
 */
bool serializer_write_json(FILE *stream, const ifm_record_t *records, size_t count);

/**
 * @brief Streams an array of normalized financial model records into a file handle as Canonical CSV.
 */
bool serializer_write_csv(FILE *stream, const ifm_record_t *records, size_t count);

#endif /* SERIALIZER_H */
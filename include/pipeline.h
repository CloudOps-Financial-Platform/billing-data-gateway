#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdbool.h>
#include <stddef.h>
#include "ifm_spec.h"

/*
 * Orchestrates the full ingestion plane:
 * 1. Maps file via mmap
 * 2. Auto-detects cloud provider schema
 * 3. Tokenizes and stream-normalizes rows zero-copy into IFM record array
 */
bool pipeline_process_file(const char *filepath, ifm_record_t *records_buffer,
                           size_t max_records, size_t *out_records_count);

#endif /* PIPELINE_H */
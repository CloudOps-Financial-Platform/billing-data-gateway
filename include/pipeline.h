#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdbool.h>
#include <stddef.h>
#include "ifm_spec.h"
#include "mmap_reader.h" // Include to ensure mmap_file_t is visible

/**
 * @brief Zero-copy pipeline parsing abstraction core.
 * @note mfile structure must remain allocated and open outside this execution scope.
 */
bool pipeline_process_file(mmap_file_t *mfile, ifm_record_t *records_buffer,
                           size_t max_records, size_t *out_records_count);

#endif
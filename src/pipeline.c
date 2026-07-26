#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pipeline.h"
#include "mmap_reader.h"
#include "provider_registry.h"

#define MAX_ROW_TOKENS 256

/* Fast in-place comma tokenizer operating on string slices (Handles trailing tokens cleanly) */
static size_t tokenize_line(const char *line_start, size_t line_len, str_slice_t *tokens, size_t max_tokens)
{
    if (!line_start || line_len == 0 || !tokens)
        return 0;

    size_t count = 0;
    const char *ptr = line_start;
    const char *start = ptr;
    const char *end = line_start + line_len;

    while (ptr <= end && count < max_tokens)
    {
        if (ptr == end || *ptr == ',' || *ptr == '\n' || *ptr == '\r')
        {
            size_t len = ptr - start;
            if (len > 0 && start[len - 1] == '\r')
            {
                len--;
            }
            tokens[count].ptr = start;
            tokens[count].len = len;
            count++;

            if (ptr == end || *ptr == '\n' || *ptr == '\r')
            {
                break;
            }
            start = ptr + 1;
        }
        ptr++;
    }

    return count;
}

bool pipeline_process_file(const char *filepath, ifm_record_t *records_buffer,
                           size_t max_records, size_t *out_records_count)
{
    if (!filepath || !records_buffer || !out_records_count || max_records == 0)
    {
        return false;
    }

    *out_records_count = 0;

    /* 1. Initialize Memory-Mapped File Buffer */
    mmap_file_t mfile;
    if (!mmap_open(filepath, &mfile))
    {
        return false;
    }

    if (mfile.size == 0)
    {
        mmap_close(&mfile);
        return false;
    }

    /* 2. Initialize Provider Registry */
    provider_registry_init();

    /* 3. Extract Header Line */
    const char *buffer = (const char *)mfile.data;
    const char *header_end = memchr(buffer, '\n', mfile.size);
    if (!header_end)
    {
        mmap_close(&mfile);
        return false;
    }

    size_t header_len = header_end - buffer;
    char header_buf[2048];
    if (header_len >= sizeof(header_buf))
        header_len = sizeof(header_buf) - 1;
    memcpy(header_buf, buffer, header_len);
    header_buf[header_len] = '\0';

    /* 4. Auto-Detect Provider Schema */
    const provider_adapter_t *adapter = provider_registry_detect(header_buf);
    if (!adapter)
    {
        mmap_close(&mfile);
        return false; /* Unrecognized vendor layout */
    }

    /* 5. Resolve Column Mapping Table */
    canonical_col_map_t col_map;
    if (!adapter->resolve_headers(header_buf, &col_map))
    {
        mmap_close(&mfile);
        return false;
    }

    /* 6. Stream Rows Zero-Copy */
    const char *row_start = header_end + 1;
    const char *file_end = buffer + mfile.size;
    size_t current_line = 2;
    str_slice_t row_tokens[MAX_ROW_TOKENS];

    while (row_start < file_end && *out_records_count < max_records)
    {
        const char *row_end = memchr(row_start, '\n', file_end - row_start);
        size_t row_len = row_end ? (size_t)(row_end - row_start) : (size_t)(file_end - row_start);

        /* Skip empty lines */
        if (row_len > 0 && *row_start != '\r')
        {
            size_t token_count = tokenize_line(row_start, row_len, row_tokens, MAX_ROW_TOKENS);

            ifm_record_t *rec = &records_buffer[*out_records_count];
            rec->source_line = current_line;

            if (adapter->parse_row(row_tokens, token_count, &col_map, rec))
            {
                (*out_records_count)++;
            }
        }

        if (!row_end)
            break;
        row_start = row_end + 1;
        current_line++;
    }

    /* 7. Clean Up Memory Map */
    mmap_close(&mfile);
    return true;
}
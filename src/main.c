#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/ifm_core.h"
#include "../include/mmap_reader.h"
#include "../include/csv_parser.h"
#include "../include/provider_adapters.h"

#define MAX_RECORDS 100000
#define MAX_TOKENS 64

static uint64_t get_time_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main(int argc, char **argv)
{
    const char *filepath = (argc > 1) ? argv[1] : "data/enterprise_billing.csv";

    mmap_file_t file;
    if (!mmap_open(filepath, &file))
    {
        fprintf(stderr, "[FATAL] Failed to open file: %s\n", filepath);
        return 1;
    }

    ifm_record_t *record_buffer = malloc(sizeof(ifm_record_t) * MAX_RECORDS);
    if (!record_buffer)
    {
        fprintf(stderr, "[FATAL] Memory allocation failed\n");
        mmap_close(&file);
        return 1;
    }

    const char *curr = file.data;
    const char *end = file.data + file.size;
    const char *line_start = curr;

    // 1. Scan and Parse Header Line Dynamically
    while (curr < end && *curr != '\n')
        curr++;
    str_slice_t header_tokens[MAX_TOKENS];
    size_t header_token_count = csv_tokenize_line(line_start, curr, header_tokens, MAX_TOKENS);

    header_map_t hmap = parse_csv_header(header_tokens, header_token_count);

    // STRICT INVARIANT: Hard-abort pipeline if vital header fields are missing
    if (!hmap.is_valid)
    {
        fprintf(stderr, "=================================================================\n");
        fprintf(stderr, "[FATAL ABORT] Ingestion halted: CSV Header missing required fields.\n");
        fprintf(stderr, "=================================================================\n");
        free(record_buffer);
        mmap_close(&file);
        return 1; // ABORT EXECUTABLE IMMEDIATELY
    }

    curr++; // Skip newline after header
    line_start = curr;

    size_t total_rows_scanned = 0;
    size_t successful_parses = 0;
    size_t parse_errors = 0;

    uint64_t start_time = get_time_ns();

    // 2. Stream Rows Using Dynamic Header Map
    while (curr < end && successful_parses < MAX_RECORDS)
    {
        if (*curr == '\n' || curr == end - 1)
        {
            const char *line_end = (*curr == '\n') ? curr : curr + 1;

            if (line_end > line_start)
            {
                total_rows_scanned++;

                str_slice_t tokens[MAX_TOKENS];
                size_t token_count = csv_tokenize_line(line_start, line_end, tokens, MAX_TOKENS);

                ifm_status_e status = normalize_with_header_map(tokens, token_count, hmap, successful_parses + 1, &record_buffer[successful_parses]);

                if (status == IFM_OK)
                {
                    successful_parses++;
                }
                else
                {
                    parse_errors++;
                    if (parse_errors <= 5)
                    {
                        fprintf(stderr, "[DIAGNOSTIC] Row %zu Quarantined: %s\n", total_rows_scanned, ifm_status_to_string(status));
                    }
                }
            }

            line_start = curr + 1;
        }
        curr++;
    }

    uint64_t end_time = get_time_ns();
    mmap_close(&file);

    double total_time_ms = (double)(end_time - start_time) / 1e6;

    printf("=================================================================\n");
    printf("   CLOUDOPS FINANCIAL PLATFORM — BILLING DATA GATEWAY v0.3.2     \n");
    printf("=================================================================\n");
    printf(" Engine Mode       : Dynamic Schema Resolution (Milestone 3.2)\n");
    printf(" File Processed    : %s\n", filepath);
    printf(" Columns Mapped    : %zu Fields Resolved Dynamically\n", header_token_count);
    printf(" Total Rows Read   : %zu\n", total_rows_scanned);
    printf(" Successful IFM    : %zu\n", successful_parses);
    printf(" Quarantined Rows  : %zu\n", parse_errors);
    printf(" Dataset Pass Rate : %.2f%%\n", total_rows_scanned > 0 ? ((double)successful_parses / total_rows_scanned) * 100.0 : 0.0);
    printf(" Ingestion Time    : %.4f ms\n", total_time_ms);
    printf("=================================================================\n");

    if (successful_parses > 0)
    {
        printf(" Sample IFM Record [1] -> Provider: %d | Account: %s | Service: %s | Cost: $%.2f\n",
               record_buffer[0].provider,
               record_buffer[0].account_id,
               record_buffer[0].service_name,
               record_buffer[0].billed_cost);
        printf("=================================================================\n");
    }

    free(record_buffer);
    return 0;
}
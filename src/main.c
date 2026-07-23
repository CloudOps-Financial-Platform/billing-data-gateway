#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/ifm_core.h"
#include "../include/mmap_reader.h"
#include "../include/csv_parser.h"
#include "../include/provider_adapters.h"

#define MAX_RECORDS 100000
#define MAX_TOKENS 32

static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main(int argc, char** argv) {
    const char* filepath = (argc > 1) ? argv[1] : "data/enterprise_billing.csv";

    mmap_file_t file;
    if (!mmap_open(filepath, &file)) {
        fprintf(stderr, "[FATAL] Failed to memory-map billing log: %s\n", filepath);
        return 1;
    }

    ifm_record_t* record_buffer = malloc(sizeof(ifm_record_t) * MAX_RECORDS);
    if (!record_buffer) {
        fprintf(stderr, "[FATAL] Memory allocation failed\n");
        mmap_close(&file);
        return 1;
    }

    size_t total_rows = 0;
    size_t successful_parses = 0;
    size_t parse_errors = 0;

    uint64_t start_time = get_time_ns();

    const char* curr = file.data;
    const char* end = file.data + file.size;
    const char* line_start = curr;

    while (curr < end && *curr != '\n') curr++;
    curr++;
    line_start = curr;

    while (curr < end && successful_parses < MAX_RECORDS) {
        if (*curr == '\n' || curr == end - 1) {
            const char* line_end = (*curr == '\n') ? curr : curr + 1;
            total_rows++;

            str_slice_t tokens[MAX_TOKENS];
            size_t token_count = csv_tokenize_line(line_start, line_end, tokens, MAX_TOKENS);

            ifm_status_e status = normalize_raw_tokens(tokens, token_count, successful_parses + 1, &record_buffer[successful_parses]);

            if (status == IFM_OK) {
                successful_parses++;
            } else {
                parse_errors++;
            }

            line_start = curr + 1;
        }
        curr++;
    }

    uint64_t end_time = get_time_ns();
    mmap_close(&file);

    double total_time_ms = (double)(end_time - start_time) / 1e6;
    double MB_processed = (double)file.size / (1024.0 * 1024.0);
    double throughput_mbs = (total_time_ms > 0) ? (MB_processed / (total_time_ms / 1000.0)) : 0;
    double rows_per_sec = (total_time_ms > 0) ? ((double)successful_parses / (total_time_ms / 1000.0)) : 0;

    printf("=================================================================\n");
    printf("   CLOUDOPS FINANCIAL PLATFORM — BILLING DATA GATEWAY v1.0       \n");
    printf("=================================================================\n");
    printf(" Data Engine Mode   : POSIX mmap() Zero-Copy + Zero-Allocation Parser\n");
    printf(" File Ingested      : %s (%.2f MB)\n", filepath, MB_processed);
    printf(" Total Rows Scanned : %zu\n", total_rows);
    printf(" Normalized Records : %zu\n", successful_parses);
    printf(" Parse Errors       : %zu\n", parse_errors);
    printf(" Correctness Rate   : %.2f%%\n", total_rows > 0 ? ((double)successful_parses / total_rows) * 100.0 : 0.0);
    printf(" Ingestion Time     : %.4f ms\n", total_time_ms);
    printf(" Throughput Rate    : %.2f MB/sec | %.0f Rows/sec\n", throughput_mbs, rows_per_sec);
    printf("=================================================================\n");

    if (successful_parses > 0) {
        printf(" Sample IFM Record [1] -> Provider: %d | Account: %s | Service: %s | Cost: $%.2f\n",
               record_buffer[0].provider,
               record_buffer[0].account_id,
               record_buffer[0].service_name,
               record_buffer[0].billed_cost);
    }
    printf("=================================================================\n");

    free(record_buffer);
    return 0;
}
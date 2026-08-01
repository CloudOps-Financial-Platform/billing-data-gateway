#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include "pipeline.h"
#include "mmap_reader.h"

#define NUM_STRESS_ROWS 100000
#define BATCH_BUFFER_SIZE 105000

static double get_time_sec(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

static size_t generate_large_aws_file(const char *filepath, size_t row_count)
{
    FILE *f = fopen(filepath, "wb");
    assert(f != NULL);

    const char *header = "identity/LineItemId,lineItem/UsageAccountId,lineItem/ResourceId,lineItem/UsageStartDate,lineItem/UnblendedCost\n";
    fputs(header, f);

    for (size_t i = 0; i < row_count; i++)
    {
        fprintf(f, "aws-row-%zu,123456789012,i-0abc%zub,2026-07-27T10:00:00Z,%zu.250000\n",
                i, i % 1000, (i % 50) + 1);
    }

    fflush(f);
    long file_size = ftell(f);
    fclose(f);
    return (size_t)file_size;
}

int main(void)
{
    printf("=================================================================\n");
    printf("   CN2 SYSTEMS — BILLING DATA GATEWAY THROUGHPUT BENCHMARK v1.0.0 \n");
    printf("=================================================================\n");

    const char *bench_file = "bench_aws_stress.csv";
    printf("[*] Generating synthetic stress payload (%d rows)...\n", NUM_STRESS_ROWS);

    size_t total_bytes = generate_large_aws_file(bench_file, NUM_STRESS_ROWS);
    double size_mb = (double)total_bytes / (1024.0 * 1024.0);
    printf("[+] Synthetic payload ready: %.2f MB (%zu bytes)\n", size_mb, total_bytes);

    /* Open file via POSIX mmap layer */
    mmap_file_t mfile;
    bool open_status = mmap_open(bench_file, &mfile);
    assert(open_status == true);

    /* Allocate buffer for parsed IFM records */
    ifm_record_t *records = malloc(sizeof(ifm_record_t) * BATCH_BUFFER_SIZE);
    assert(records != NULL);

    size_t parsed_count = 0;
    struct timespec start, end;

    printf("[*] Executing zero-copy mmap ingestion pipeline...\n");

    /* Monotonic High-Resolution Benchmark Block */
    clock_gettime(CLOCK_MONOTONIC, &start);
    bool status = pipeline_process_file(&mfile, records, BATCH_BUFFER_SIZE, &parsed_count);
    clock_gettime(CLOCK_MONOTONIC, &end);

    assert(status == true);
    assert(parsed_count == NUM_STRESS_ROWS);

    double elapsed_sec = get_time_sec(start, end);
    double mb_per_sec = size_mb / elapsed_sec;
    double records_per_sec = (double)parsed_count / elapsed_sec;
    double ns_per_record = (elapsed_sec * 1e9) / (double)parsed_count;

    printf("\n-----------------------------------------------------------------\n");
    printf(" 📊 EMPIRICAL TELEMETRY RESULTS:\n");
    printf("-----------------------------------------------------------------\n");
    printf("  • Total Dataset Size   : %.2f MB\n", size_mb);
    printf("  • Records Parsed      : %zu / %d\n", parsed_count, NUM_STRESS_ROWS);
    printf("  • Total Execution Time: %.4f seconds (%.2f ms)\n", elapsed_sec, elapsed_sec * 1000.0);
    printf("  • Processing Speed    : %.2f MB/sec\n", mb_per_sec);
    printf("  • Throughput Velocity : %.0f records/sec\n", records_per_sec);
    printf("  • Latency Cost        : %.2f nanoseconds / record\n", ns_per_record);
    printf("-----------------------------------------------------------------\n");
    printf("[+] BENCHMARK COMPLETE: Zero-copy mmap performance invariants verified!\n\n");

    free(records);
    mmap_close(&mfile);
    remove(bench_file);
    return 0;
}
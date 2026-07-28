#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "pipeline.h"
#include "version.h"

#define MAX_CLI_RECORDS 500000 // Safe bounded operational frame allocation

static void print_usage(const char *prog_name)
{
    fprintf(stderr, "Billing Data Gateway — High-Performance Cloud Cost Normalization\n\n");
    fprintf(stderr, "Usage: %s -i <input_csv_path> [-v]\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i <path>  Specify the path to the raw cloud billing CSV export file\n");
    fprintf(stderr, "  -v         Display engine version and build metadata strings\n");
}

int main(int argc, char *argv[])
{
    char *input_path = NULL;
    int opt;

    /* Parse execution argument vector flags using POSIX getopt */
    while ((opt = getopt(argc, argv, "i:v")) != -1)
    {
        switch (opt)
        {
        case 'i':
            input_path = optarg;
            break;
        case 'v':
            printf("Billing Data Gateway\n");
            printf("Version : %s\n", GATEWAY_VERSION);
            printf("Build   : %s\n", GATEWAY_BUILD_DATE);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Enforce mandatory parameter perimeters */
    if (!input_path)
    {
        fprintf(stderr, "[!] Operational Error: Missing mandatory input parameter (-i).\n\n");
        print_usage(argv[0]);
        return 1;
    }

    /* Allocate localized record buffer array on heap */
    ifm_record_t *records = malloc(sizeof(ifm_record_t) * MAX_CLI_RECORDS);
    if (!records)
    {
        fprintf(stderr, "[!] Critical Error: Heap frame allocation failure for %d records.\n", MAX_CLI_RECORDS);
        return 1;
    }

    size_t out_count = 0;
    struct timespec start, end;

    /* Invoke Low-Latency Memory-Mapped Pipeline */
    clock_gettime(CLOCK_MONOTONIC, &start);
    bool status = pipeline_process_file(input_path, records, MAX_CLI_RECORDS, &out_count);
    clock_gettime(CLOCK_MONOTONIC, &end);

    if (!status)
    {
        fprintf(stderr, "[!] Ingestion Intercept: Parsing failure or unrecognized schema on target file.\n");
        free(records);
        return 1;
    }

    double duration_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    /* High-Density, Professional Telemetry Output Block */
    printf("\n=================================================================\n");
    printf(" 📊 INGESTION TELEMETRY DISPATCH SUCCESSFUL\n");
    printf("=================================================================\n");
    printf("  • Target Input File  : %s\n", input_path);
    printf("  • Rows Processed     : %zu records\n", out_count);
    printf("  • Ingestion Runtime  : %.2f ms (%.4f seconds)\n", duration_sec * 1000.0, duration_sec);
    printf("  • Core Throughput    : %.0f rows/sec\n", (double)out_count / duration_sec);
    printf("  • Gateway Exit Code  : 0\n");
    printf("=================================================================\n\n");

    free(records);
    return 0;
}
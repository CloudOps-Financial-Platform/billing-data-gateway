#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include "pipeline.h"
#include "version.h"

#define MAX_CLI_RECORDS 500000

static void print_usage(const char *prog_name)
{
    fprintf(stderr, "Billing Data Gateway — High-Performance Cloud Cost Normalization\n\n");
    fprintf(stderr, "Usage: %s -i <input_csv_path> [-v] [-h]\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i <path>  Specify the path to the raw cloud billing CSV export file\n");
    fprintf(stderr, "  -v         Display engine version and build metadata strings\n");
    fprintf(stderr, "  -h         Display this help menu\n");
}

static const char *get_provider_string(provider_type_t type)
{
    switch (type)
    {
    case PROVIDER_AWS_CUR:
        return "AWS (CUR 2.0)";
    case PROVIDER_AZURE_COST:
        return "Azure (Cost Management)";
    default:
        return "UNKNOWN / UNRESOLVED";
    }
}

int main(int argc, char *argv[])
{
    char *input_path = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "i:vh")) != -1)
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
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!input_path)
    {
        fprintf(stderr, "[!] Operational Error: Missing mandatory input parameter (-i).\n\n");
        print_usage(argv[0]);
        return 1;
    }

    struct stat st;
    if (stat(input_path, &st) != 0)
    {
        fprintf(stderr, "[!] File Error: Failed to retrieve stats or file does not exist at: %s\n", input_path);
        return 1;
    }
    double file_size_mb = (double)st.st_size / (1024.0 * 1024.0);

    ifm_record_t *records = malloc(sizeof(ifm_record_t) * MAX_CLI_RECORDS);
    if (!records)
    {
        fprintf(stderr, "[!] Critical Error: Heap frame allocation failure.\n");
        return 1;
    }

    /* Initialize memory array buffer cleanly to prevent garbage readbacks */
    memset(records, 0, sizeof(ifm_record_t) * MAX_CLI_RECORDS);

    size_t out_count = 0;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    bool status = pipeline_process_file(input_path, records, MAX_CLI_RECORDS, &out_count);
    clock_gettime(CLOCK_MONOTONIC, &end);

    /* HARD CHECKPOINT: Reject zero-row records or false status flags immediately */
    if (!status || out_count == 0)
    {
        fprintf(stderr, "[!] Ingestion Intercept: Parsing pipeline aborted safely due to structural errors.\n");
        fprintf(stderr, "[!] Gateway Exit Code  : 1\n");
        free(records);
        return 1;
    }

    double duration_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    const char *detected_provider = get_provider_string(records[0].provider);

    printf("\n====================================================\n");
    printf("Billing Data Gateway %s\n", GATEWAY_VERSION);
    printf("====================================================\n");
    printf("Input File : %s (%.3f MB)\n", input_path, file_size_mb);
    printf("Provider   : %s\n", detected_provider);
    printf("Rows       : %zu\n", out_count);
    printf("Runtime    : %.2f ms (%.4f seconds)\n", duration_sec * 1000.0, duration_sec);
    printf("Throughput : %.0f rows/sec\n", (double)out_count / duration_sec);
    printf("\nExit Code  : 0\n");
    printf("====================================================\n\n");

    free(records);
    return 0;
}
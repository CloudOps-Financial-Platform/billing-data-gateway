#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include "pipeline.h"
#include "mmap_reader.h"
#include "serializer.h"
#include "version.h"

#define MAX_CLI_RECORDS 500000

static void print_usage(const char *prog_name)
{
    fprintf(stderr, "Billing Data Gateway — High-Performance Cloud Cost Normalization\n\n");
    fprintf(stderr, "Usage: %s -i <input_csv_path> [-f <format>] [-v] [-h]\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i <path>    Specify the path to the raw cloud billing CSV export file\n");
    fprintf(stderr, "  -f <format>  Specify output format: 'json' or 'csv' (streams raw data payload)\n");
    fprintf(stderr, "  -v           Display engine version and build metadata strings\n");
    fprintf(stderr, "  -h           Display this help menu\n\n");
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

static void format_file_size(long long bytes, char *buf, size_t buf_len)
{
    if (bytes < 1024)
    {
        snprintf(buf, buf_len, "%lld B", bytes);
    }
    else if (bytes < 1024 * 1024)
    {
        snprintf(buf, buf_len, "%.2f KB", (double)bytes / 1024.0);
    }
    else
    {
        snprintf(buf, buf_len, "%.2f MB", (double)bytes / (1024.0 * 1024.0));
    }
}

int main(int argc, char *argv[])
{
    char *input_path = NULL;
    char *format_arg = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "i:f:vh")) != -1)
    {
        switch (opt)
        {
        case 'i':
            input_path = optarg;
            break;
        case 'f':
            format_arg = optarg;
            break;
        case 'v':
            printf("Billing Data Gateway v%s\n", GATEWAY_VERSION);
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
        fprintf(stderr, "[!] Operational Error: Missing mandatory input parameter (-i).\n");
        return 1;
    }

    struct stat st;
    if (stat(input_path, &st) != 0)
    {
        fprintf(stderr, "[!] File Error: Failed to access path: %s\n", input_path);
        return 1;
    }

    char size_str[32];
    format_file_size(st.st_size, size_str, sizeof(size_str));

    mmap_file_t mfile;
    if (!mmap_open(input_path, &mfile))
    {
        fprintf(stderr, "[!] Mmap Error: Failed mapping memory space for %s\n", input_path);
        return 1;
    }

    ifm_record_t *records = malloc(sizeof(ifm_record_t) * MAX_CLI_RECORDS);
    if (!records)
    {
        fprintf(stderr, "[!] Critical Error: Memory initialization break.\n");
        mmap_close(&mfile);
        return 1;
    }
    memset(records, 0, sizeof(ifm_record_t) * MAX_CLI_RECORDS);

    size_t out_count = 0;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    bool status = pipeline_process_file(&mfile, records, MAX_CLI_RECORDS, &out_count);
    clock_gettime(CLOCK_MONOTONIC, &end);

    if (!status || out_count == 0)
    {
        fprintf(stderr, "[!] Ingestion Failure: Pipeline parsing checks failed out.\n");
        free(records);
        mmap_close(&mfile);
        return 1;
    }

    /* Output Format Dispatch Layer */
    if (format_arg)
    {
        if (strcmp(format_arg, "json") == 0)
        {
            serializer_write_json(stdout, records, out_count);
            free(records);
            mmap_close(&mfile);
            return 0;
        }
        else if (strcmp(format_arg, "csv") == 0)
        {
            serializer_write_csv(stdout, records, out_count);
            free(records);
            mmap_close(&mfile);
            return 0;
        }
        else
        {
            fprintf(stderr, "[!] Format Error: Unsupported format '%s'. Supported formats: 'json', 'csv'\n", format_arg);
            free(records);
            mmap_close(&mfile);
            return 1;
        }
    }

    double duration_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\n====================================================\n");
    printf("Billing Data Gateway %s\n", GATEWAY_VERSION);
    printf("====================================================\n");
    printf("Input File        : %s\n", input_path);
    printf("File Size         : %s\n", size_str);
    printf("Provider          : %s\n", get_provider_string(records[0].provider));
    printf("Records Processed : %zu\n", out_count);
    printf("Elapsed Time      : %.2f ms\n", duration_sec * 1000.0);
    printf("Throughput        : %.0f rows/sec\n", (double)out_count / duration_sec);
    printf("====================================================\n\n");

    free(records);
    mmap_close(&mfile);
    return 0;
}
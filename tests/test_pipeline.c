#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pipeline.h"
#include "mmap_reader.h"
#include "validation.h"

static void create_mock_aws_file(const char *filepath)
{
    FILE *f = fopen(filepath, "wb");
    assert(f != NULL);

    /* AWS CUR 2.0 Mock Header & Valid Sample Data */
    const char *data =
        "identity/LineItemId,lineItem/UsageAccountId,lineItem/ResourceId,lineItem/UsageStartDate,lineItem/UnblendedCost\n"
        "aws-row-101,123456789012,i-0abc1234z,2026-07-26T00:00:00Z,45.250000\n"
        "aws-row-102,123456789012,i-0xyz5678w,2026-07-26T01:00:00Z,0.007120\n";

    fwrite(data, 1, strlen(data), f);
    fflush(f);
    fclose(f);
}

static void create_mock_malformed_file(const char *filepath)
{
    FILE *f = fopen(filepath, "wb");
    assert(f != NULL);

    /* AWS CUR 2.0 Mock Header & Malformed Data (Missing Account, Empty Resource, Invalid Date) */
    const char *data =
        "identity/LineItemId,lineItem/UsageAccountId,lineItem/ResourceId,lineItem/UsageStartDate,lineItem/UnblendedCost\n"
        "aws-row-err,,,not_a_date,0.000000\n";

    fwrite(data, 1, strlen(data), f);
    fflush(f);
    fclose(f);
}

int main(void)
{
    printf("[*] Running Pipeline End-to-End Verification Harness...\n");

    /* ------------------------------------------------------------- */
    /* Test 1: Valid Dataset Parsing and Data Invariants           */
    /* ------------------------------------------------------------- */
    const char *mock_path = "mock_aws_billing.csv";
    create_mock_aws_file(mock_path);

    mmap_file_t mfile;
    bool open_status = mmap_open(mock_path, &mfile);
    assert(open_status == true);

    ifm_record_t buffer[10];
    size_t parsed_count = 0;

    bool status = pipeline_process_file(&mfile, buffer, 10, &parsed_count);

    if (!status)
    {
        fprintf(stderr, "[!] CRITICAL: Pipeline returned failure status.\n");
        mmap_close(&mfile);
        remove(mock_path);
        return 1;
    }

    assert(parsed_count == 2);
    assert(buffer[0].provider == PROVIDER_AWS_CUR);
    assert(buffer[0].flags == RECORD_VALID);
    assert(buffer[0].billed_cost_micros == 45250000LL); /* $45.25 */
    assert(buffer[1].billed_cost_micros == 7120LL);     /* $0.007120 */

    mmap_close(&mfile);
    remove(mock_path);

    /* ------------------------------------------------------------- */
    /* Test 2: System 8 Validation Engine Flag Assertions            */
    /* ------------------------------------------------------------- */
    const char *mock_err_path = "mock_aws_err.csv";
    create_mock_malformed_file(mock_err_path);

    bool err_open_status = mmap_open(mock_err_path, &mfile);
    assert(err_open_status == true);

    ifm_record_t err_buffer[10];
    size_t err_parsed_count = 0;

    bool err_status = pipeline_process_file(&mfile, err_buffer, 10, &err_parsed_count);

    assert(err_status == true);
    assert(err_parsed_count == 1);

    /* Assert System 5 validation bitmask flags match expected violations (flags = 13) */
    assert(err_buffer[0].flags != RECORD_VALID);
    assert((err_buffer[0].flags & RECORD_MISSING_KEY) != 0);
    assert((err_buffer[0].flags & RECORD_INVALID_DATE) != 0);
    assert((err_buffer[0].flags & RECORD_EMPTY_RESOURCE) != 0);

    mmap_close(&mfile);
    remove(mock_err_path);

    printf("[+] SUCCESS: Pipeline executed, zero-copy string slices valid, validation assertions verified!\n");
    return 0;
}
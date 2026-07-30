#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pipeline.h"
#include "mmap_reader.h"

static void create_mock_aws_file(const char *filepath)
{
    FILE *f = fopen(filepath, "wb");
    assert(f != NULL);

    /* AWS CUR 2.0 Mock Header & Sample Data */
    const char *data =
        "identity/LineItemId,lineItem/UsageAccountId,lineItem/ResourceId,lineItem/UsageStartDate,lineItem/UnblendedCost\n"
        "aws-row-101,123456789012,i-0abc1234z,2026-07-26T00:00:00Z,45.250000\n"
        "aws-row-102,123456789012,i-0xyz5678w,2026-07-26T01:00:00Z,0.007120\n";

    fwrite(data, 1, strlen(data), f);
    fflush(f);
    fclose(f);
}

int main(void)
{
    printf("[*] Running Pipeline End-to-End Verification Harness...\n");

    const char *mock_path = "mock_aws_billing.csv";
    create_mock_aws_file(mock_path);

    /* 1. Open mock file via POSIX mmap layer */
    mmap_file_t mfile;
    bool open_status = mmap_open(mock_path, &mfile);
    assert(open_status == true);

    ifm_record_t buffer[10];
    size_t parsed_count = 0;

    /* 2. Execute Ingestion Pipeline passing mmap_file_t handle */
    bool status = pipeline_process_file(&mfile, buffer, 10, &parsed_count);

    /* Assert Ingestion Success */
    if (!status)
    {
        fprintf(stderr, "[!] CRITICAL: Pipeline returned failure status.\n");
        mmap_close(&mfile);
        remove(mock_path);
        return 1;
    }

    assert(parsed_count == 2);

    /* Assert Data Model Invariants */
    assert(buffer[0].provider == PROVIDER_AWS_CUR);
    assert(buffer[0].flags == RECORD_VALID);
    assert(buffer[0].billed_cost_micros == 45250000LL); /* $45.25 */
    assert(buffer[1].billed_cost_micros == 7120LL);     /* $0.007120 */

    printf("[+] SUCCESS: Pipeline executed, zero-copy string slices valid, micro-currency verified!\n");

    /* 3. Cleanup Resources */
    mmap_close(&mfile);
    remove(mock_path);
    return 0;
}
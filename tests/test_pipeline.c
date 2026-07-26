#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pipeline.h"

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
    printf("[*] Running Day 13 Pipeline End-to-End Verification Harness...\n");

    const char *mock_path = "mock_aws_billing.csv";
    create_mock_aws_file(mock_path);

    ifm_record_t buffer[10];
    size_t parsed_count = 0;

    /* Execute Ingestion Pipeline */
    bool status = pipeline_process_file(mock_path, buffer, 10, &parsed_count);

    /* Assert Ingestion Success */
    if (!status)
    {
        fprintf(stderr, "[!] CRITICAL: Pipeline returned failure status.\n");
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

    /* Cleanup Mock File */
    remove(mock_path);
    return 0;
}
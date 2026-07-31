#include "validation.h"

void validation_evaluate_record(ifm_record_t *record)
{
    if (!record)
        return;

    /* Rule 1: Check Resource / Asset ID presence */
    if (record->resource_id.len == 0 || !record->resource_id.ptr)
    {
        record->flags |= RECORD_EMPTY_RESOURCE;
    }

    /* Rule 2: ISO-8601 Timestamp Validation (Length + leading numeric year digit check) */
    if (record->usage_start_raw.len < 10 || !record->usage_start_raw.ptr ||
        !(record->usage_start_raw.ptr[0] >= '0' && record->usage_start_raw.ptr[0] <= '9'))
    {
        record->flags |= RECORD_INVALID_DATE;
    }

    /* Rule 3: Account ID Presence Check */
    if (record->account_id.len == 0 || !record->account_id.ptr)
    {
        record->flags |= RECORD_MISSING_KEY;
    }
}
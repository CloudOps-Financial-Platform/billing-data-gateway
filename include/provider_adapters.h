#ifndef PROVIDER_ADAPTERS_H
#define PROVIDER_ADAPTERS_H

#include "../include/ifm_core.h"

// Dynamic Column Offset Map Structure
typedef struct
{
    int idx_provider;
    int idx_account_id;
    int idx_service_name;
    int idx_region;
    int idx_timestamp_start;
    int idx_timestamp_end;
    int idx_billed_cost;
    int idx_effective_cost;
    int idx_usage_quantity;
    bool is_valid;
} header_map_t;

// Legacy signature left untouched to keep system compiling
ifm_status_e normalize_raw_tokens(str_slice_t *tokens, size_t token_count, uint64_t record_id, ifm_record_t *out_record);

#endif // PROVIDER_ADAPTERS_H
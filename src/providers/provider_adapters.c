#include "../include/provider_adapters.h"
#include "../include/csv_parser.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

typedef struct
{
    const char *aliases[8];
    size_t alias_count;
} column_definition_t;

// Data-Driven Multi-Cloud Schema Lookup Matrix
static const column_definition_t schema_matrix[9] = {
    {{"provider", "vendor", NULL}, 2},
    {{"account_id", "lineItem/AccountId", "SubscriptionId", NULL}, 3},
    {{"service_name", "lineItem/ProductCode", "ServiceName", NULL}, 3},
    {{"region", "product/region", "ResourceLocation", NULL}, 3},
    {{"start_time", "lineItem/UsageStartDate", "Date", NULL}, 3},
    {{"end_time", "lineItem/UsageEndDate", NULL}, 2},
    {{"billed_cost", "lineItem/UnblendedCost", "CostInBillingCurrency", "cost", NULL}, 4},
    {{"effective_cost", "lineItem/BlendedCost", NULL}, 2},
    {{"usage_quantity", "lineItem/UsageAmount", "Quantity", NULL}, 3}};

static bool slice_matches_aliases(str_slice_t slice, column_definition_t col)
{
    for (size_t i = 0; i < col.alias_count; i++)
    {
        if (slice.len == strlen(col.aliases[i]))
        {
            if (strncasecmp(slice.ptr, col.aliases[i], slice.len) == 0)
            {
                return true;
            }
        }
    }
    return false;
}

header_map_t parse_csv_header(str_slice_t *tokens, size_t token_count)
{
    header_map_t map = {
        .idx_provider = -1, .idx_account_id = -1, .idx_service_name = -1, .idx_region = -1, .idx_timestamp_start = -1, .idx_timestamp_end = -1, .idx_billed_cost = -1, .idx_effective_cost = -1, .idx_usage_quantity = -1, .is_valid = false};

    size_t unknown_count = 0;

    int *map_fields[] = {
        &map.idx_provider,
        &map.idx_account_id,
        &map.idx_service_name,
        &map.idx_region,
        &map.idx_timestamp_start,
        &map.idx_timestamp_end,
        &map.idx_billed_cost,
        &map.idx_effective_cost,
        &map.idx_usage_quantity};

    for (size_t i = 0; i < token_count; i++)
    {
        int target_idx = -1;

        if (slice_matches_aliases(tokens[i], schema_matrix[0]))
            target_idx = 0;
        else if (slice_matches_aliases(tokens[i], schema_matrix[1]))
            target_idx = 1;
        else if (slice_matches_aliases(tokens[i], schema_matrix[2]))
            target_idx = 2;
        else if (slice_matches_aliases(tokens[i], schema_matrix[3]))
            target_idx = 3;
        else if (slice_matches_aliases(tokens[i], schema_matrix[4]))
            target_idx = 4;
        else if (slice_matches_aliases(tokens[i], schema_matrix[5]))
            target_idx = 5;
        else if (slice_matches_aliases(tokens[i], schema_matrix[6]))
            target_idx = 6;
        else if (slice_matches_aliases(tokens[i], schema_matrix[7]))
            target_idx = 7;
        else if (slice_matches_aliases(tokens[i], schema_matrix[8]))
            target_idx = 8;

        if (target_idx == -1)
        {
            unknown_count++;
            continue;
        }

        if (*map_fields[target_idx] != -1)
        {
            fprintf(stderr, "[WARNING] Duplicate schema column detected at index %zu. Overwriting.\n", i);
        }
        *map_fields[target_idx] = (int)i;
    }

    if (unknown_count > 0)
    {
        printf("[DIAGNOSTIC] Header processing complete. Skipped %zu unmapped operational columns.\n", unknown_count);
    }

    // Anchor Validation Guard
    if (map.idx_provider >= 0 && map.idx_billed_cost >= 0)
    {
        map.is_valid = true;
    }
    else
    {
        if (map.idx_provider == -1)
            fprintf(stderr, "[ERROR] Crucial schema field missing: 'provider'\n");
        if (map.idx_billed_cost == -1)
            fprintf(stderr, "[ERROR] Crucial schema field missing: 'billed_cost'\n");
    }

    return map;
}

ifm_status_e normalize_with_header_map(str_slice_t *tokens, size_t token_count, header_map_t map, uint64_t record_id, ifm_record_t *out_record)
{
    if (!tokens || !out_record || !map.is_valid)
    {
        return IFM_ERR_NULL_POINTER;
    }

    out_record->record_id = record_id;

    // 1. Dynamic Provider Extraction
    if (map.idx_provider >= 0 && (size_t)map.idx_provider < token_count)
    {
        char provider_str[16];
        slice_to_cstring(tokens[map.idx_provider], provider_str, sizeof(provider_str));

        if (strncasecmp(provider_str, "AWS", 3) == 0)
        {
            out_record->provider = PROVIDER_AWS;
        }
        else if (strncasecmp(provider_str, "AZURE", 5) == 0)
        {
            out_record->provider = PROVIDER_AZURE;
        }
        else if (strncasecmp(provider_str, "GCP", 3) == 0)
        {
            out_record->provider = PROVIDER_GCP;
        }
        else
        {
            out_record->provider = PROVIDER_UNKNOWN;
            return IFM_ERR_UNSUPPORTED_PROVIDER;
        }
    }

    // 2. Dynamic Structural Text Extraction
    if (map.idx_account_id >= 0 && (size_t)map.idx_account_id < token_count)
        slice_to_cstring(tokens[map.idx_account_id], out_record->account_id, sizeof(out_record->account_id));

    if (map.idx_service_name >= 0 && (size_t)map.idx_service_name < token_count)
        slice_to_cstring(tokens[map.idx_service_name], out_record->service_name, sizeof(out_record->service_name));

    if (map.idx_region >= 0 && (size_t)map.idx_region < token_count)
        slice_to_cstring(tokens[map.idx_region], out_record->region, sizeof(out_record->region));

    // 3. Dynamic Safe Numeric Conversions
    if (map.idx_timestamp_start >= 0 && (size_t)map.idx_timestamp_start < token_count)
        safe_slice_to_uint64(tokens[map.idx_timestamp_start], &out_record->timestamp_start);

    if (map.idx_timestamp_end >= 0 && (size_t)map.idx_timestamp_end < token_count)
        safe_slice_to_uint64(tokens[map.idx_timestamp_end], &out_record->timestamp_end);

    if (map.idx_billed_cost >= 0 && (size_t)map.idx_billed_cost < token_count)
    {
        if (!safe_slice_to_double(tokens[map.idx_billed_cost], &out_record->billed_cost))
        {
            return IFM_ERR_INVALID_NUMERIC_COST;
        }
    }

    if (map.idx_effective_cost >= 0 && (size_t)map.idx_effective_cost < token_count)
        safe_slice_to_double(tokens[map.idx_effective_cost], &out_record->effective_cost);

    if (map.idx_usage_quantity >= 0 && (size_t)map.idx_usage_quantity < token_count)
        safe_slice_to_double(tokens[map.idx_usage_quantity], &out_record->usage_quantity);

    return IFM_OK;
}
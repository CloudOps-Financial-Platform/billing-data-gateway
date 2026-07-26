#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "provider_adapters.h"
#include "utils/fixed_point.h"

static bool azure_detect(const char *header_line)
{
    if (!header_line)
        return false;
    return (strstr(header_line, "costInBillingCurrency") != NULL ||
            strstr(header_line, "SubscriptionId") != NULL ||
            strstr(header_line, "PreTaxCost") != NULL);
}

static bool azure_resolve_headers(const char *header_line, canonical_col_map_t *col_map)
{
    if (!header_line || !col_map)
        return false;

    col_map->row_id_idx = -1;
    col_map->account_id_idx = -1;
    col_map->resource_id_idx = -1;
    col_map->usage_start_idx = -1;
    col_map->cost_idx = -1;

    int16_t current_col = 0;
    const char *ptr = header_line;
    const char *start = ptr;

    while (1)
    {
        if (*ptr == ',' || *ptr == '\n' || *ptr == '\r' || *ptr == '\0')
        {
            size_t len = ptr - start;
            if (len > 0)
            {
                if (strncmp(start, "ResourceId", len) == 0 && len == strlen("ResourceId"))
                    col_map->row_id_idx = current_col;
                else if (strncmp(start, "SubscriptionId", len) == 0 && len == strlen("SubscriptionId"))
                    col_map->account_id_idx = current_col;
                else if (strncmp(start, "MeterId", len) == 0 && len == strlen("MeterId"))
                    col_map->resource_id_idx = current_col;
                else if (strncmp(start, "Date", len) == 0 && len == strlen("Date"))
                    col_map->usage_start_idx = current_col;
                else if ((strncmp(start, "costInBillingCurrency", len) == 0 && len == strlen("costInBillingCurrency")) ||
                         (strncmp(start, "PreTaxCost", len) == 0 && len == strlen("PreTaxCost")))
                    col_map->cost_idx = current_col;
            }

            current_col++;
            start = ptr + 1;

            if (*ptr == '\0' || *ptr == '\n' || *ptr == '\r')
            {
                break;
            }
        }
        ptr++;
    }

    return (col_map->cost_idx != -1);
}

static bool azure_parse_row(const str_slice_t *tokens, size_t token_count,
                            const canonical_col_map_t *col_map, ifm_record_t *out_record)
{
    if (!tokens || !col_map || !out_record)
        return false;

    out_record->provider = PROVIDER_AZURE_COST;
    out_record->flags = RECORD_VALID;

    if (col_map->row_id_idx >= 0 && (size_t)col_map->row_id_idx < token_count)
        out_record->provider_row_id = tokens[col_map->row_id_idx];

    if (col_map->account_id_idx >= 0 && (size_t)col_map->account_id_idx < token_count)
        out_record->account_id = tokens[col_map->account_id_idx];

    if (col_map->resource_id_idx >= 0 && (size_t)col_map->resource_id_idx < token_count)
        out_record->resource_id = tokens[col_map->resource_id_idx];

    if (col_map->usage_start_idx >= 0 && (size_t)col_map->usage_start_idx < token_count)
        out_record->usage_start_raw = tokens[col_map->usage_start_idx];

    if (col_map->cost_idx >= 0 && (size_t)col_map->cost_idx < token_count)
    {
        out_record->billed_cost_micros = parse_cost_micros(&tokens[col_map->cost_idx]);
    }
    else
    {
        out_record->flags |= RECORD_MISSING_KEY;
    }

    return true;
}

const provider_adapter_t azure_adapter = {
    .type = PROVIDER_AZURE_COST,
    .name = "Azure Cost Management Adapter",
    .detect = azure_detect,
    .resolve_headers = azure_resolve_headers,
    .parse_row = azure_parse_row};
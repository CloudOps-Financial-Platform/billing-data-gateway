#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "provider_adapters.h"
#include "utils/fixed_point.h"

static bool aws_detect(const char *header_line)
{
    if (!header_line)
        return false;
    return (strstr(header_line, "lineItem/UnblendedCost") != NULL ||
            strstr(header_line, "identity/LineItemId") != NULL);
}

static bool aws_resolve_headers(const char *header_line, canonical_col_map_t *col_map)
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
                if (strncmp(start, "identity/LineItemId", len) == 0 && len == strlen("identity/LineItemId"))
                    col_map->row_id_idx = current_col;
                else if (strncmp(start, "lineItem/UsageAccountId", len) == 0 && len == strlen("lineItem/UsageAccountId"))
                    col_map->account_id_idx = current_col;
                else if (strncmp(start, "lineItem/ResourceId", len) == 0 && len == strlen("lineItem/ResourceId"))
                    col_map->resource_id_idx = current_col;
                else if (strncmp(start, "lineItem/UsageStartDate", len) == 0 && len == strlen("lineItem/UsageStartDate"))
                    col_map->usage_start_idx = current_col;
                else if (strncmp(start, "lineItem/UnblendedCost", len) == 0 && len == strlen("lineItem/UnblendedCost"))
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

static bool aws_parse_row(const str_slice_t *tokens, size_t token_count,
                          const canonical_col_map_t *col_map, ifm_record_t *out_record)
{
    if (!tokens || !col_map || !out_record)
        return false;

    out_record->provider = PROVIDER_AWS_CUR;
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

const provider_adapter_t aws_adapter = {
    .type = PROVIDER_AWS_CUR,
    .name = "AWS CUR 2.0 Adapter",
    .detect = aws_detect,
    .resolve_headers = aws_resolve_headers,
    .parse_row = aws_parse_row};
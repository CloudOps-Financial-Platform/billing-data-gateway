#include "../include/provider_adapters.h"
#include "../include/csv_parser.h"
#include <string.h>
#include <strings.h>

typedef struct
{
    const char *aliases[8];
    size_t alias_count;
} column_definition_t;

// Data-driven multi-cloud schema lookup matrix
static const column_definition_t schema_matrix[] = {
    [0] = {.aliases = {"provider", "vendor"}, .alias_count = 2},
    [1] = {.aliases = {"account_id", "lineItem/AccountId", "SubscriptionId"}, .alias_count = 3},
    [2] = {.aliases = {"service_name", "lineItem/ProductCode", "ServiceName"}, .alias_count = 3},
    [3] = {.aliases = {"region", "product/region", "ResourceLocation"}, .alias_count = 3},
    [4] = {.aliases = {"start_time", "lineItem/UsageStartDate", "Date"}, .alias_count = 3},
    [5] = {.aliases = {"end_time", "lineItem/UsageEndDate"}, .alias_count = 2},
    [6] = {.aliases = {"billed_cost", "lineItem/UnblendedCost", "CostInBillingCurrency", "cost"}, .alias_count = 4},
    [7] = {.aliases = {"effective_cost", "lineItem/BlendedCost"}, .alias_count = 2},
    [8] = {.aliases = {"usage_quantity", "lineItem/UsageAmount", "Quantity"}, .alias_count = 3}};

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
#ifndef IFM_CORE_H
#define IFM_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum
{
    PROVIDER_UNKNOWN = 0,
    PROVIDER_AWS,
    PROVIDER_AZURE,
    PROVIDER_GCP
} ifm_provider_e;

// Detailed Diagnostic Error Codes
typedef enum
{
    IFM_OK = 0,
    IFM_ERR_EMPTY_FILE,
    IFM_ERR_NULL_POINTER,
    IFM_ERR_INSUFFICIENT_FIELDS,
    IFM_ERR_UNSUPPORTED_PROVIDER,
    IFM_ERR_INVALID_NUMERIC_COST,
    IFM_ERR_INVALID_TIMESTAMP,
    IFM_ERR_TRUNCATED_ROW
} ifm_status_e;

typedef struct
{
    uint64_t record_id;
    ifm_provider_e provider;
    char account_id[64];
    char service_name[64];
    char region[32];
    uint64_t timestamp_start;
    uint64_t timestamp_end;
    double billed_cost;
    double effective_cost;
    double usage_quantity;
} ifm_record_t;

typedef struct
{
    const char *ptr;
    size_t len;
} str_slice_t;

// Human-readable error string converter
const char *ifm_status_to_string(ifm_status_e status);

#endif // IFM_CORE_H
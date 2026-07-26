#ifndef IFM_SPEC_H
#define IFM_SPEC_H

#include <stdint.h>
#include <stddef.h>

/* Zero-copy string pointer slice into mmap buffer */
typedef struct
{
    const char *ptr;
    size_t len;
} str_slice_t;

/* Supported provider targets */
typedef enum
{
    PROVIDER_UNKNOWN = 0,
    PROVIDER_AWS_CUR,
    PROVIDER_AZURE_COST,
    PROVIDER_GCP_BILLING
} provider_type_t;

/* Ingestion record state flags for production debugging */
typedef enum
{
    RECORD_VALID = 0,
    RECORD_MALFORMED = 1 << 0,
    RECORD_MISSING_KEY = 1 << 1,
    RECORD_OVERFLOW = 1 << 2
} record_flags_t;

/* Intermediate Financial Model (IFM) Record Struct */
typedef struct
{
    /* Metadata */
    size_t source_line;       /* Row offset in source file */
    provider_type_t provider; /* Source provider enum */
    record_flags_t flags;     /* Processing status flags */

    /* Identity & Time (Preserved as raw slices) */
    str_slice_t provider_row_id; /* Raw vendor row ID */
    str_slice_t account_id;      /* Vendor account / subscription ID */
    str_slice_t resource_id;     /* Asset ARN / Resource ID */
    str_slice_t usage_start_raw; /* Raw timestamp string slice */

    /* Financial Metrics (Fixed-Point Arithmetic) */
    int64_t billed_cost_micros; /* Cost in micro-units (1 USD = 1,000,000) */
} ifm_record_t;

/* Column index map (Supports up to 32,767 columns) */
typedef struct
{
    int16_t row_id_idx;
    int16_t account_id_idx;
    int16_t resource_id_idx;
    int16_t usage_start_idx;
    int16_t cost_idx;
} canonical_col_map_t;

#endif /* IFM_SPEC_H */
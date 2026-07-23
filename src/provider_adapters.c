#include "../include/provider_adapters.h"
#include "../include/csv_parser.h"
#include <string.h>

ifm_status_e normalize_raw_tokens(str_slice_t* tokens, size_t token_count, uint64_t record_id, ifm_record_t* out_record) {
    if (!tokens || !out_record || token_count < 9) {
        return IFM_ERR_MALFORMED_ROW;
    }

    out_record->record_id = record_id;

    char provider_str[16];
    slice_to_cstring(tokens[0], provider_str, sizeof(provider_str));

    if (strncmp(provider_str, "AWS", 3) == 0) {
        out_record->provider = PROVIDER_AWS;
    } else if (strncmp(provider_str, "AZURE", 5) == 0) {
        out_record->provider = PROVIDER_AZURE;
    } else if (strncmp(provider_str, "GCP", 3) == 0) {
        out_record->provider = PROVIDER_GCP;
    } else {
        out_record->provider = PROVIDER_UNKNOWN;
        return IFM_ERR_UNSUPPORTED_PROVIDER;
    }

    slice_to_cstring(tokens[1], out_record->account_id, sizeof(out_record->account_id));
    slice_to_cstring(tokens[2], out_record->service_name, sizeof(out_record->service_name));
    slice_to_cstring(tokens[3], out_record->region, sizeof(out_record->region));

    out_record->timestamp_start = slice_to_uint64(tokens[4]);
    out_record->timestamp_end   = slice_to_uint64(tokens[5]);
    out_record->billed_cost     = slice_to_double(tokens[6]);
    out_record->effective_cost  = slice_to_double(tokens[7]);
    out_record->usage_quantity  = slice_to_double(tokens[8]);

    return IFM_OK;
}
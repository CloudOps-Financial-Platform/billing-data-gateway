#ifndef PROVIDER_ADAPTERS_H
#define PROVIDER_ADAPTERS_H

#include "../include/ifm_core.h"

ifm_status_e normalize_raw_tokens(str_slice_t* tokens, size_t token_count, uint64_t record_id, ifm_record_t* out_record);

#endif // PROVIDER_ADAPTERS_H
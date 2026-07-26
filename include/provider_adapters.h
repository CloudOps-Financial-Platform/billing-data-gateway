#ifndef PROVIDER_ADAPTERS_H
#define PROVIDER_ADAPTERS_H

#include <stdbool.h>
#include "ifm_spec.h"

/* Provider Adapter Interface Contract */
typedef struct
{
    provider_type_t type;
    const char *name;

    /* Inspects header row for vendor-specific signature strings */
    bool (*detect)(const char *header_line);

    /* Populates column index lookup table based on header tokens */
    bool (*resolve_headers)(const char *header_line, canonical_col_map_t *col_map);

    /* Parses a single tokenized row into a normalized IFM record */
    bool (*parse_row)(const str_slice_t *tokens, size_t token_count,
                      const canonical_col_map_t *col_map, ifm_record_t *out_record);
} provider_adapter_t;

#endif /* PROVIDER_ADAPTERS_H */
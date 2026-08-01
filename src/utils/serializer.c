#include "serializer.h"
#include <inttypes.h>

static const char *get_provider_json_name(provider_type_t type)
{
    switch (type)
    {
    case PROVIDER_AWS_CUR:
        return "AWS_CUR";
    case PROVIDER_AZURE_COST:
        return "AZURE_COST";
    default:
        return "UNKNOWN";
    }
}

static void print_json_slice(FILE *stream, str_slice_t slice)
{
    if (slice.ptr != NULL && slice.len > 0)
    {
        if (slice.len > 4096)
            return;
        fprintf(stream, "%.*s", (int)slice.len, slice.ptr);
    }
}

bool serializer_write_json(FILE *stream, const ifm_record_t *records, size_t count)
{
    if (!stream || !records || count == 0)
    {
        return false;
    }

    fprintf(stream, "[\n");

    for (size_t i = 0; i < count; i++)
    {
        const ifm_record_t *r = &records[i];

        /* Unpack fixed-point currency integers using integer division */
        int64_t dollars = r->billed_cost_micros / 1000000;
        int64_t micros = r->billed_cost_micros % 1000000;
        if (micros < 0)
        {
            micros = -micros;
        }

        fprintf(stream, "  {\n");
        fprintf(stream, "    \"source_line\": %zu,\n", r->source_line);
        fprintf(stream, "    \"provider\": \"%s\",\n", get_provider_json_name(r->provider));
        fprintf(stream, "    \"flags\": %u,\n", (unsigned int)r->flags);

        fprintf(stream, "    \"account_id\": \"");
        print_json_slice(stream, r->account_id);
        fprintf(stream, "\",\n");

        fprintf(stream, "    \"resource_id\": \"");
        print_json_slice(stream, r->resource_id);
        fprintf(stream, "\",\n");

        fprintf(stream, "    \"usage_start_raw\": \"");
        print_json_slice(stream, r->usage_start_raw);
        fprintf(stream, "\",\n");

        fprintf(stream, "    \"billed_cost\": %" PRId64 ".%06" PRId64 "\n", dollars, micros);

        fprintf(stream, "  }%s\n", (i + 1 < count) ? "," : "");
    }

    fprintf(stream, "]\n");
    return true;
}

bool serializer_write_csv(FILE *stream, const ifm_record_t *records, size_t count)
{
    if (!stream || !records || count == 0)
    {
        return false;
    }

    /* Print Canonical CSV Header */
    fprintf(stream, "source_line,provider,flags,account_id,resource_id,usage_start_raw,billed_cost\n");

    for (size_t i = 0; i < count; i++)
    {
        const ifm_record_t *r = &records[i];

        int64_t dollars = r->billed_cost_micros / 1000000;
        int64_t micros = r->billed_cost_micros % 1000000;
        if (micros < 0)
        {
            micros = -micros;
        }

        fprintf(stream, "%zu,%s,%u,", r->source_line, get_provider_json_name(r->provider), (unsigned int)r->flags);

        print_json_slice(stream, r->account_id);
        fprintf(stream, ",");

        print_json_slice(stream, r->resource_id);
        fprintf(stream, ",");

        print_json_slice(stream, r->usage_start_raw);
        fprintf(stream, ",");

        fprintf(stream, "%" PRId64 ".%06" PRId64 "\n", dollars, micros);
    }

    return true;
}
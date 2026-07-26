#include <stddef.h>
#include "../../include/provider_registry.h"

/* Externally defined adapter singletons */
extern const provider_adapter_t aws_adapter;
extern const provider_adapter_t azure_adapter;

static const provider_adapter_t *g_registry[MAX_REGISTERED_PROVIDERS];
static size_t g_registry_count = 0;

bool provider_registry_register(const provider_adapter_t *adapter)
{
    if (!adapter || g_registry_count >= MAX_REGISTERED_PROVIDERS)
    {
        return false;
    }
    g_registry[g_registry_count++] = adapter;
    return true;
}

void provider_registry_init(void)
{
    g_registry_count = 0;
    provider_registry_register(&aws_adapter);
    provider_registry_register(&azure_adapter);
}

const provider_adapter_t *provider_registry_detect(const char *header_line)
{
    if (!header_line)
        return NULL;

    for (size_t i = 0; i < g_registry_count; i++)
    {
        if (g_registry[i]->detect && g_registry[i]->detect(header_line))
        {
            return g_registry[i];
        }
    }
    return NULL;
}
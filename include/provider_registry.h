#ifndef PROVIDER_REGISTRY_H
#define PROVIDER_REGISTRY_H

#include "provider_adapters.h"

#define MAX_REGISTERED_PROVIDERS 8

/* Register a new vendor adapter into the runtime system */
bool provider_registry_register(const provider_adapter_t *adapter);

/* Inspects a header row and returns the matching adapter instance, or NULL */
const provider_adapter_t *provider_registry_detect(const char *header_line);

/* Initializes built-in adapters (AWS, Azure, etc.) */
void provider_registry_init(void);

#endif /* PROVIDER_REGISTRY_H */
#ifndef PACKAGING_PARAMS_H
#define PACKAGING_PARAMS_H

#include <stdbool.h>

typedef enum drmKeyProvider
{
    ezNoKey,
    ezRawKey,
    ezWidevine,
    ezPlayReady
} drmKeyProvider_t;

typedef enum drmProtectionScheme
{
    ez_CENC,
    ez_CENS,
    ez_CBC1,
    ez_CBCS
} drmProtectionScheme_t;

typedef struct drmParams
{
    drmKeyProvider_t keyProvider; // e.g. KeyProvider::kRawKey

    drmProtectionScheme_t protection_scheme; // e.g. EncryptionParams::kProtectionSchemeCbcs

    char pssh[2000]; // hex string, e.g. "000000317073736800000000EDEF8BA979D64ACEA3C827DCD51D21ED00000011220F7465737420636F6E74656E74206964"

    char iv[100]; // hex string, e.g. "73fbe3277bdf0bfc5217125bde4ca589"

    char key_id[100]; // hex string, e.g. "abba271e8bcf552bbd2e86a434a9a5d9"

    char key[100]; // hex string, e.g. "69eaa802a6763af979e8d1940fb88392"
} drmParams_t;

// Corresponds to StreamDescriptor in Shaka
typedef struct streamDescriptor
{
    char stream_selector[20]; // e.g, "video" | "audio"
} streamDescriptor_t;

typedef struct packagingParams
{
    double segment_duration_in_seconds;

    streamDescriptor_t stream_descriptor;

    bool isProtected;

    drmParams_t drm_params;
} packagingParams_t;

#endif

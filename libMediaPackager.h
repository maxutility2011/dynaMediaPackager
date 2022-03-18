#ifndef LIB_MEDIA_PACKAGER_H
#define LIB_MEDIA_PACKAGER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include "packaging_params.h"

typedef struct packagerResult {
    int status;
    char *pBufferOut;
    int outBufferSize;
} packagerResult_t;

int processMediaData(char *inputFileName,
                        char *pBufferData,
                        size_t bufferSize,
                        char *outputFormat,
			            packagingParams_t params,
                        uint8_t **outputBufferData,
                        size_t *outputBufferSize);

#ifdef __cplusplus
}  // extern "C"
#endif
#endif

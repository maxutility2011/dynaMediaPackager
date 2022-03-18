#ifndef MEDIA_PACKAGER_INTERFACE
#define MEDIA_PACKAGER_INTERFACE

#include "packaging_params.h"

int doProcessMediaBuffer(char *inUrl,
                            char *outFormat,
			                packagingParams_t params,
                            uint8_t **ppBufferData,
                            size_t *pBufferSize);
                            
int writeBufferToMemoryFile(char *filename,
                            uint8_t *pBufferData, 
                            size_t bufferSize);

#endif

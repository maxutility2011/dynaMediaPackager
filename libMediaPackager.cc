#include "libMediaPackager.h"
#include "mediaPackagerInterface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

int processMediaData(char *inputFileName,
		     char *pBufferData,
		     size_t bufferSize,
		     char *outputFormat,
		     packagingParams_t params,
		     uint8_t **outputBufferData,
		     size_t *outputBufferSize)
{
    char *memoryFileUrl = (char *)malloc(50);
    sprintf(memoryFileUrl, "%s%s", "memory://", inputFileName);

    writeBufferToMemoryFile(inputFileName,
                            (uint8_t *)pBufferData,
                            bufferSize);

    doProcessMediaBuffer(memoryFileUrl,
                         outputFormat,
			             params,
                         outputBufferData,
                         outputBufferSize);

    free(memoryFileUrl);
    return 0;
}

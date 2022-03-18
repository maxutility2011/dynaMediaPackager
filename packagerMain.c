#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libMediaPackager.h"

int main(int argc, char** argv)
{
    if (argc < 3 || argc > 4)
    {
        printf("%s => Usage: dynaMediaPackager [input_file] [output_file] no_protection (optional)\n", __PRETTY_FUNCTION__);
	    return 0;
    }

    char oFormat[50];
    if (strstr(argv[2], ".mp4"))
    {
        strcpy(oFormat, ".mp4");
    }
    else if (strstr(argv[2], ".ts"))
    {
        strcpy(oFormat, ".ts");
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        printf("%s => Error: Failed to open input file: %s\n", __PRETTY_FUNCTION__, argv[1]);
        return 1;
    }

    unsigned char *pInputFileBuffer = (char *)malloc(200000000); // 200 MB
    if (!pInputFileBuffer)
    {
        printf("%s => Error: Failed to allocate read buffer\n", __PRETTY_FUNCTION__);
        return 1;
    }

    fseek(input, 0, SEEK_END);
    long inputFileSize = ftell(input);
    rewind(input);

    size_t bytesRead = fread(pInputFileBuffer, sizeof(char), inputFileSize, input);
    printf("%s => Info: Size of input file: %ld, bytesRead: %lu\n", __PRETTY_FUNCTION__, inputFileSize, bytesRead);
    if (ferror(input))
    {
        printf("Error: failed to read input file\n");
        return 1;
    }

    unsigned char *outputBufferData = 0;
    size_t outputBufferSize = 0;

    // Generate configuration for the packager.
    // It is application developer's job to make sure the following parameters are correct and compatible.
    packagingParams_t params;

    if (argc == 4 && !strcmp(argv[3], "no_protection"))
    {
        params.isProtected = true;
    }

    params.isProtected = (argc == 4 && !strcmp(argv[3], "no_protection"))? false : true;

    params.segment_duration_in_seconds = 4;

    strcpy(params.stream_descriptor.stream_selector, "video");

    params.drm_params.keyProvider = ezRawKey;
    params.drm_params.protection_scheme = ez_CBCS;

    strcpy(params.drm_params.pssh, "000000317073736800000000EDEF8BA979D64ACEA3C827DCD51D21ED00000011220F7465737420636F6E74656E74206964");
    strcpy(params.drm_params.iv, "73fbe3277bdf0bfc5217125bde4ca589");
    strcpy(params.drm_params.key_id, "abba271e8bcf552bbd2e86a434a9a5d9");
    strcpy(params.drm_params.key, "69eaa802a6763af979e8d1940fb88392");

    processMediaData(argv[1],
		                pInputFileBuffer,
		                inputFileSize,
		                oFormat,
		                params,
		                &outputBufferData,
		                &outputBufferSize);

    FILE *output = fopen(argv[2], "w");
    if (output == NULL)
    {
        printf("%s => Error: Failed to open output file: %s\n", __PRETTY_FUNCTION__, argv[2]);
        return 1;
    }

    size_t bytesWritten = fwrite(outputBufferData, sizeof(char), outputBufferSize, output);
    printf("%s => Info: Size of output file: %ld, bytesWritten: %lu\n", __PRETTY_FUNCTION__, outputBufferSize, bytesWritten);
    if (ferror(output))
    {
        printf("Error: failed to write output file\n");
        return 1;
    }

    free(pInputFileBuffer);
    fclose(input);
    fclose(output);
    return 0;
}

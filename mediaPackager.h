#ifndef MEDIA_PACKAGER_H
#define MEDIA_PACKAGER_H

#include <string>
#include "packager/packager.h"
#include "packager/app/stream_descriptor.h"
#include "packaging_params.h"

using namespace std;

typedef enum
{
    MFF_TS = 0,
    MFF_MP4
} media_file_format_t;

typedef enum
{
    MT_AUDIO = 0,
    MT_VIDEO,
    MT_TEXT
} media_type_t;

typedef enum
{
    no_error = 0,
    memory_alloc_error,
    packager_init_error,
    packager_process_error
} packager_status_t;

class ezMediaPackager
{
    public:
        ezMediaPackager(int logLevel);

        virtual ~ezMediaPackager();

	    // We use memoryFile (packager/file/memory_file.h) to pass media data buffer to shaka packager engine.
	    virtual packager_status_t initialize(string inUrl, media_file_format_t outFormat, packagingParams_t params);

        virtual packager_status_t process();

        virtual uint8_t *getMediaDataBuffer() 
	    { 
	        return m_mediaDataBufferStart; 
	    }

        virtual size_t getMediaDataBufferSize() 
	    { 
	        return m_mediaDataBufferSize; 
	    }

        // Callback function used when Shaka finishes media processing.
        int64_t shakaCallback(const std::string& name, 
                              const void* processedDataBuffer, 
                              uint64_t processedDataBufferSize);

    private:
        int m_logLevel;

        int m_mediaType;

        uint8_t *m_mediaDataBuffer;

        size_t m_mediaDataBufferSize;

        uint8_t *m_mediaDataBufferStart;

        shaka::Packager m_shaka;

        string m_output_file_path;
};
#endif

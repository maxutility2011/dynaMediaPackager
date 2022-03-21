#include <iostream>
#include <fstream>
#include <string>
#include <functional>
#include <memory>
#include <unistd.h>

#include "packager/file/file.h"
#include "packager/file/memory_file.h"
#include "packager/base/command_line.h"
#include "packager/file/file_closer.h"

#include "packaging_utils.h"
#include "mediaPackager.h"
#include "mediaPackagerInterface.h"

using namespace shaka;
using namespace std::placeholders;
using namespace std;

#define MAX_SHAKA_CALLBACK_BUFFER_SIZE 10000000 

#define DEFAULT_PATTERN_ENCRYPTION_CRYPT_BLOCKS_VIDEO 1 // CMAF CBCS uses 1:9 skip ratio for video segments, 1 crypt blockfollowed by 9 clear blocks 

#define DEFAULT_PATTERN_ENCRYPTION_CLEAR_BLOCKS_VIDEO 9 

dynaMediaPackager::dynaMediaPackager(int loglevel) : m_logLevel(loglevel), // TODO: logLevel is hardcoded
                                                    m_mediaDataBuffer(0),
                                                    m_mediaDataBufferSize(0)
{}

dynaMediaPackager::~dynaMediaPackager()
{}

int64_t dynaMediaPackager::shakaCallback(const string& name, 
 	                                 const void* dataBuffer, 
				         uint64_t dataBufferSize)
{
    if (m_mediaDataBuffer)
    {
        memcpy(m_mediaDataBuffer, (uint8_t *)dataBuffer, dataBufferSize);

        m_mediaDataBuffer += dataBufferSize;
        m_mediaDataBufferSize += dataBufferSize;
    }
    else
    {
        cout << "Error: null output buffer." << endl;
        return 0;
    }
  
    return dataBufferSize;
}

packager_status_t dynaMediaPackager::initialize(string inputStreamUrl, 
                                                media_file_format_t oFormat,
					        packagingParams_t params)
{
    int argc = 2; // Mock argc for packager logging purpose ONLY, NOT used in the media pipeline.
    char *argv[argc];
    argv[0] = new char[50];
    argv[1] = new char[50];
    strcpy(argv[0], "./mediaPackager"); // Mock argv for logging purpose ONLY, NOT used in the media pipeline.
    strcpy(argv[1], "input.mp4"); // Mock argv for logging purpose ONLY, NOT used in the media pipeline.
    base::CommandLine::Init(argc, argv);

    logging::LoggingSettings log_settings;
    log_settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    logging::InitLogging(log_settings);

    int logLevel;
    bool enableVerboseLogging = true; // hardcoded 
    if (enableVerboseLogging)
    {
        logLevel = -1 * logging::LOG_FATAL;
    }
    else
    {
        logLevel = logging::LOG_WARNING;
    }

    logging::SetMinLogLevel(logLevel);

    PackagingParams packaging_params;
    vector<StreamDescriptor> stream_descriptors;
    StreamDescriptor stream_descriptor;

    stream_descriptor.input = inputStreamUrl;
    stream_descriptor.stream_selector = params.stream_descriptor.stream_selector; 

    std::function<int64_t(const string&, const void*, uint64_t)> cbf = std::bind(&dynaMediaPackager::shakaCallback, this, _1, _2, _3);

    packaging_params.buffer_callback_params.write_func = cbf; 
    string mockFile;

    // Per https://github.com/shaka-project/shaka-packager/blob/main/docs/source/options/transport_stream_output_options.rst,
    // a proper value of transport_stream_timestamp_offset_ms needs to be applied when transmuxing mp4 segments to ts segment to compensate for negative timestamp.
    packaging_params.transport_stream_timestamp_offset_ms = 100;

    if (oFormat == MFF_TS)
    {
        mockFile = File::MakeCallbackFileName(
        packaging_params.buffer_callback_params, "video_$Number$.ts");

        // stream_descriptor.segment_template must be used for TS segments. 
        stream_descriptor.segment_template = mockFile;
        // stream_descriptor.output is not allowed for transport stream.
    }
    else if (oFormat == MFF_MP4)
    {
        mockFile = File::MakeCallbackFileName(packaging_params.buffer_callback_params, "output_file.mp4");
        stream_descriptor.output = mockFile;
    }

    stream_descriptors.push_back(stream_descriptor);

    ChunkingParams& chunking_params = packaging_params.chunking_params;
    chunking_params.segment_duration_in_seconds = params.segment_duration_in_seconds; 

    if (params.isProtected)
    {
    	EncryptionParams& encryption_params = packaging_params.encryption_params;
    	switch (params.drm_params.keyProvider) {
            case dynaNoKey:
                encryption_params.key_provider = KeyProvider::kNone;
            	break;
       	    case dynaRawKey:
                encryption_params.key_provider = KeyProvider::kRawKey;
                break;
            case dynaWidevine:
                encryption_params.key_provider = KeyProvider::kWidevine;
                break;
            case dynaPlayReady:
                encryption_params.key_provider = KeyProvider::kPlayReady;
                break;
            default:
                cout << "Error: unknown drm key provider type: " << params.drm_params.keyProvider << endl;
        }
    
        switch (params.drm_params.protection_scheme) {
            case dyna_CENC:
                encryption_params.protection_scheme = EncryptionParams::kProtectionSchemeCenc;
                break;
            case dyna_CENS:
                encryption_params.protection_scheme = EncryptionParams::kProtectionSchemeCbc1;
                break;
            case dyna_CBC1:
                encryption_params.protection_scheme = EncryptionParams::kProtectionSchemeCens;
                break;
            case dyna_CBCS:
                encryption_params.protection_scheme = EncryptionParams::kProtectionSchemeCbcs;
                break;
            default:
                cout << "Error: unknown drm protection scheme: " << params.drm_params.protection_scheme << endl;
        }

        // Use 1:9 skip ratio for CMAF cbcs video encryption per CENC spec (ISO/IEC 23001-1:2016) and CMAF spec (ISO/IEC 23000-19:2020)
        encryption_params.crypt_byte_block = DEFAULT_PATTERN_ENCRYPTION_CRYPT_BLOCKS_VIDEO;
        encryption_params.skip_byte_block = DEFAULT_PATTERN_ENCRYPTION_CLEAR_BLOCKS_VIDEO;

        string pssh_string = params.drm_params.pssh;
        vector<uint8_t> pssh_vector = HexToBytes(pssh_string);
        encryption_params.raw_key.pssh = pssh_vector;

        string iv_string = params.drm_params.iv;
        vector<uint8_t> iv_vector = HexToBytes(iv_string);
        encryption_params.raw_key.iv = iv_vector;

        RawKeyParams::KeyInfo& key_info = encryption_params.raw_key.key_map[""];

        string key_id_string = params.drm_params.key_id;
        vector<uint8_t> key_id_vector = HexToBytes(key_id_string);
        key_info.key_id = key_id_vector;

        string key_string = params.drm_params.key;
        vector<uint8_t> key_vector = HexToBytes(key_string);

        key_info.key = key_vector;
    }

    shaka::Status status = m_shaka.Initialize(packaging_params,
                                              stream_descriptors);
    if (!status.ok()) 
    {
        cout << "Error: Failed to initialize shaka packager! Error: " << status.ToString() << endl;
        return packager_init_error;
    }

    m_mediaDataBuffer = new uint8_t[MAX_SHAKA_CALLBACK_BUFFER_SIZE]; 
    if (!m_mediaDataBuffer)
    {
        cout << "Error: Failed to allocate shaka callback buffer!" << endl;
        return memory_alloc_error;
    }

    m_mediaDataBufferStart = m_mediaDataBuffer;
    return no_error;
}

packager_status_t dynaMediaPackager::process()
{
    Status status = m_shaka.Run();
    if (!status.ok()) 
    {
        cout << "Error: Failed to run shaka packager! Error: " << status.ToString() << endl;
        return packager_process_error;
    }

    return no_error;
}

int doProcessMediaBuffer(char *memoryFileUrl,
                         char *outputFormat,
			 packagingParams_t params,
                         uint8_t **ppBufferData,
                         size_t *pBufferSize)
{
    packager_status_t status = no_error;
    dynaMediaPackager packager(0);

    media_file_format_t oFormat;
    if (!string(outputFormat).compare(".ts"))
    {
        oFormat = MFF_TS;
    }
    else if (!string(outputFormat).compare(".mp4"))
    {
        oFormat = MFF_MP4;
    }
    else
    {
        cout << "Error: Unknown media file format: " << outputFormat << endl;
        return 1;
    }

    status = packager.initialize(string(memoryFileUrl), oFormat, params); 
    if (status != no_error)
    {
        *ppBufferData = 0;
        *pBufferSize = 0;
        return 1;
    }

    packager.process();
    if (status != no_error)
    {
        *ppBufferData = 0;
        *pBufferSize = 0;
        return 1;
    }

    // Seems like shaka.Run() always waits for the packaging worker (its child thread) to finish,
    // such that packager.process() bceoms a blocking function call. As a result, the following 
    // call getMediaDataBuffer() will always see full output media buffer. There is no need to add 
    // additional logic to wait for shakaCallback to finish.
    *ppBufferData = packager.getMediaDataBuffer();
    *pBufferSize = packager.getMediaDataBufferSize();
   
    return 0;
}

int writeBufferToMemoryFile(char *filename,
                            uint8_t *pBufferData, 
                            size_t bufferSize)
{
    string memoryFileUrl = "memory://";
    memoryFileUrl += filename;
    LOG(INFO) << "Writing memory file: '" << filename << "'.";

    std::unique_ptr<File, FileCloser> writer(File::Open(memoryFileUrl.c_str(), "w"));

    writer->Write(pBufferData, bufferSize);

    // Shaka MemoryFile implementation does not allow multiple openings of a same file, even for concurrent read/write. So, we must close the file after writing.
    writer.release()->Close(); 
    return 0;
}

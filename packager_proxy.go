package main

/*

#cgo LDFLAGS: -L./ -L../shaka_packager/src/out/Release/ -L../shaka_packager/src/out/Release/obj/base/ -L../shaka_packager/src/out/Release/obj/third_party/ -L../shaka_packager/src/out/Release/obj/file/ -L../shaka_packager/src/out/Release/obj/base/third_party/libevent/ -L../shaka_packager/src/out/Release/obj/base/third_party/dynamic_annotations/ -L../shaka_packager/src/out/Release/obj/ -L../shaka_packager/src/out/Release/obj/hls/ -L../shaka_packager/src/out/Release/obj/media/ -L../shaka_packager/src/out/Release/obj/media/base/ -L../shaka_packager/src/out/Release/obj/third_party/protobuf/ -L../shaka_packager/src/out/Release/obj/third_party/libxml/ -L../shaka_packager/src/out/Release/obj/third_party/zlib/ -L../shaka_packager/src/out/Release/obj/mpd/ -L../shaka_packager/src/out/Release/obj/media/chunking/ -L../shaka_packager/src/out/Release/obj/media/codecs/ -L../shaka_packager/src/out/Release/obj/media/crypto/ -L../shaka_packager/src/out/Release/obj/media/demuxer/ -L../shaka_packager/src/out/Release/obj/media/formats/ -L../shaka_packager/src/out/Release/obj/third_party/libpng/ -L../shaka_packager/src/out/Release/obj/media/formats/mp4/ -L../shaka_packager/src/out/Release/obj/media/event/ -L../shaka_packager/src/out/Release/obj/media/origin/ -L../shaka_packager/src/out/Release/obj/media/formats/packed_audio/ -L../shaka_packager/src/out/Release/obj/media/formats/mp2t/ -L../shaka_packager/src/out/Release/obj/media/formats/dvb/ -L../shaka_packager/src/out/Release/obj/media/formats/ttml/ -L../shaka_packager/src/out/Release/obj/third_party/zlib/ -L../shaka_packager/src/out/Release/obj/third_party/boringssl/ -L../shaka_packager/src/out/Release/obj/third_party/icu/ -L../shaka_packager/src/out/Release/obj/third_party/modp_b64/ -L../shaka_packager/src/out/Release/obj/third_party/gflags/ -L../shaka_packager/src/out/Release/obj/version/ -L../shaka_packager/src/out/Release/obj/third_party/curl/ -L../shaka_packager/src/out/Release/obj/media/formats/webm/ -L../shaka_packager/src/out/Release/obj/third_party/libwebm/ -L../shaka_packager/src/out/Release/obj/media/event/ -L../shaka_packager/src/out/Release/obj/media/formats/ -L../shaka_packager/src/out/Release/obj/media/formats/webvtt/ -L../shaka_packager/src/out/Release/obj/media/trick_play/ -L../shaka_packager/src/out/Release/obj/media/formats/wvm/ -L../shaka_packager/src/out/Release/obj/media/replicator/ -L/usr/lib/x86_64-linux-gnu/ -lMediaPackager -lpackager -lcrypto -lttml -lmpd_builder -lhls_builder -lfile -levent -lwidevine_pssh_data_proto -lstatus -lpacked_audio -ltrick_play -lorigin -ldemuxer -lmp4 -lxml2 -licuuc -lchrome_zlib -lzlib_x86_simd -lgflags -lmp2t -lwebm -lmedia_event -lcodecs -ldvb -lpng -lmedia_base -lbase -lbase_static -lmodp_b64 -lwidevine_common_encryption_proto -levent -lversion -lsymbolize -lmanifest_base -lwebvtt -lwvm -lreplicator -lchunking -lmedia_info_proto -lprotobuf_full_do_not_use -lmkvmuxer -lcurl -lboringssl -lboringssl_asm -lrt -ldl -latomic -lm -lpthread -lstdc++ -lcares

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libMediaPackager.h"

packagerResult_t packageMediaSegment(char *fileName,
                                        char *pInputBufferData,
			                            size_t inputBufferSize)
{
    packagingParams_t params;
    params.isProtected = false;
    params.segment_duration_in_seconds = 4;
    strcpy(params.stream_descriptor.stream_selector, "video");

    uint8_t *outputBufferData = 0;
    size_t outputBufferSize = 0;

    int rc = processMediaData(fileName,
                              pInputBufferData,
                              inputBufferSize,
                              ".ts",
                              params,
                              &outputBufferData,
                              &outputBufferSize);

    packagerResult_t res;
    res.status = rc;
    res.pBufferOut = (char *)outputBufferData;
    res.outBufferSize = outputBufferSize;

    return res;
}
*/
import "C"

import (
    "fmt"
    "net/http"
    "strconv"
    "io/ioutil"
    "strings"
    "unsafe"
    "log"
)

func main() {
    http.HandleFunc("/", packager_handler)

    fmt.Println("Packager proxy listening on port ", 8080)
    log.Fatal(http.ListenAndServe(":8080", nil))
}

var initMp4Buffer = ""

func downloadInitSegment() int {
    initSegmentUrl := "http://localhost:80/media/init.mp4"
    resp, err := http.Get(initSegmentUrl)
    if err != nil {
        fmt.Println("Error: Failed to download: ", initSegmentUrl)
        return 1
    }

    defer resp.Body.Close()

    bodyBytes, err := ioutil.ReadAll(resp.Body)
    if err != nil {
        fmt.Println("Error: Failed to read response body")
        return 1
    }

    responseBodyString := string(bodyBytes)
    initMp4Buffer = responseBodyString
    return 0
}

func packager_handler(w http.ResponseWriter, r *http.Request) {
    posLastSlash := strings.LastIndex(r.URL.Path, "/")
    objUrl := r.URL.Path[posLastSlash + 1 :]

    posLastDotInObjUrl := strings.LastIndex(objUrl, ".")
    requestedFileExtension := objUrl[posLastDotInObjUrl :]
    requestedFileName := objUrl[: posLastDotInObjUrl]

    fmt.Println("objUrl: ", objUrl)
    fmt.Println("requestedFileExtension: ", requestedFileExtension)
    fmt.Println("requestedFileName: ", requestedFileName)

    objUrl_new := ""
    // Clients request ts segments, origin server only has fmp4 segments as mezzanine data.
    // The proxy needs to download fmp4 segments from the origin,
    // repackages into ts segments, and sens back to clients.
    if requestedFileExtension == ".ts" {
	    objUrl_new = requestedFileName + ".m4v" // mezzanine media data are stored as .m4v segments
    } else if requestedFileExtension == ".m3u8" {
	    fmt.Println("Info: pass thru m3u8")
	    objUrl_new = objUrl
    } else if requestedFileExtension == ".m4v" || requestedFileExtension == ".mp4" {
	    fmt.Println("Info: pass thru m4v or mp4")
        objUrl_new = objUrl
    } else {
	    fmt.Println("Error: Unsupported container format: ", requestedFileExtension)
	    http.Error(w, "Internal Server Errors.", 500)
        return
    }

    originObjUrl := "http://localhost:80/media/" + objUrl_new
    fmt.Println(originObjUrl)

    // Download fmp4 media segments from the origin server.
    resp, err := http.Get(originObjUrl) 
    if err != nil {
        panic(err)
    }

    defer resp.Body.Close()
    if resp.StatusCode != http.StatusOK {
        fmt.Println("Error: Failed to download objects from origin: ", originObjUrl, ", response code: ", resp.StatusCode)
	http.Error(w, "Internal Server Errors.", 500)
        return
    }

    // Read media segment data from http get response.
    bodyBytes, err := ioutil.ReadAll(resp.Body)
    if err != nil {
        fmt.Println("Error: Failed to read response body")
        http.Error(w, "Internal Server Errors.", 500)
        return
    }

    var responseDataBuffer []byte // All the media segment responses are sent from memory buffer directly.

    // The streaming client requested ts segments
    if requestedFileExtension == ".ts" {
	    responseBodyString := string(bodyBytes)

        // To repackage from fmp4 to ts, the fmp4 initialization segment is also needed.
        // The proxy downloads the init segment, if not yet done.
        if initMp4Buffer == "" {
            err := downloadInitSegment()
            if err == 1 {
                fmt.Println("Error: Failed to download init segment")
                http.Error(w, "Internal Server Errors.", 500)
                return
            }
        }

        // Concatenate the init segment with the media segment.
        var concatenatedSegment []byte
        if initMp4Buffer != "" {
            concatenatedSegment = append(concatenatedSegment, initMp4Buffer...)
            concatenatedSegment = append(concatenatedSegment, responseBodyString...)
        }

        // Now, call ezMediaPackager API to convert from fmp4 to ts.
        concatenatedSegmentCString := C.CString(string(concatenatedSegment))
        concatenatedSegmentStringLength := C.ulong(len(string(concatenatedSegment)))

        requestedFileNameCString := C.CString(requestedFileName)
        res := C.packageMediaSegment(requestedFileNameCString, concatenatedSegmentCString, concatenatedSegmentStringLength)
        defer C.free(unsafe.Pointer(requestedFileNameCString))
        defer C.free(unsafe.Pointer(concatenatedSegmentCString))

        if res.status == 1 {
            fmt.Println("Error: Failed to repackage media segment")
            http.Error(w, "Internal Server Errors.", 500)
            return
        }

        var C_Buffer *C.char
        C_Buffer = res.pBufferOut
        defer C.free(unsafe.Pointer(C_Buffer))

        responseDataBuffer = C.GoBytes(unsafe.Pointer(C_Buffer), res.outBufferSize)
    } else if requestedFileExtension == ".mp4" || requestedFileExtension == ".m4v" {
        responseDataBuffer = bodyBytes
    } else if requestedFileExtension == ".m3u8" {
        responseDataBuffer = bodyBytes
    }

    var FileContentType string
    if requestedFileExtension == ".ts" {
        FileContentType = "video/mp2t"
    } else if requestedFileExtension == ".mp4" || requestedFileExtension == ".m4v" {
        FileContentType = "video/mp4"
    } else if requestedFileExtension == ".m3u8" {
        FileContentType = "application/x-mpegurl"
    }

    w.Header().Set("Content-Type", FileContentType)
    w.Header().Set("Content-Length", strconv.FormatInt(int64(len(responseDataBuffer)), 10))

    w.Write(responseDataBuffer)
}

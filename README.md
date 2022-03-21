# dynaMediaPackager
This repo provides a C/C++ library called dynaMediaPackager for transmuxing and encrypting media data. The library provides one single function which accepts a raw memory buffer that stores for example mp4 or ts media data, run all the media processing jobs, then returns the processed media data back to the caller function. 

The library is developed on top of the Shaka packager API. However, unlike running Shaka packager directly on a media file which requires reading the input file and then followed by writing the output to another file, client applications of dynaMediaPackager can pass in input media data as memory buffer, and after the media processing are done, the applications receive processed media data also from memory buffer. This API design provides two performance benefits, 

    - Eliminate the need to read input and write output when using Shaka packager directly on file inputs, hence reduce I/O overhead and improve media packaging throughput.

    - The client applications receives processed media data as memory buffer. It can run further media processing on the returned data all in memory. If the client application is a Just-in-Time media packager (aka. dynamic packager) built into a webserver, the processed media data can be sent back to web clients (e.g. HLS or DASH players) as HTTP response.  

Structure of this repo:

    - The .cc files provide the C++ implementation of dynaMediaPackager.

    - The .c files provide C interface for integrating the library into programs   written in C, or programs written in Go which use CGO to call C functions. For example, dynaMediaPackager can be integrated into NGINX (written in C) as a module to implement a dynamic media packager.

    - packagerMain.cc provides a C/C++ sample application that uses dynaMediaPackager.

    - packager_proxy.go provides a Go program that implements a simple dynamic packager proxy that can play both an HLS-fmp4 stream and an HLS-ts stream at the same time, from only one single set of mezzanine fmp4 media segments. The fmp4 segments are dynamically repackaged to ts segments to stream as an HLS-ts stream.

How to build:

To build dynaMediaPackager, first check out Shaka packager source from https://github.com/google/shaka-packager, put it in the same directory as this repo. Follow https://github.com/shaka-project/shaka-packager/blob/main/docs/source/build_instructions.md to build Shaka packager on Ubuntu. Build Shaka packager for release, "ninja -C out/Release". Depending on its release version, Shaka packager may have slightly different dependencies. You may need to resolve the dependencies on your own. For example, I had to manually install libcares. Then, cd to dynaMediaSegmentPackager/, run "make" to build the C++ sample packager, and the Go dynamic packager. 

Run "dynaMediaPackager [input] [output]" to perform media segment repackaging and encryption, run "./dynaMediaPackager [input] [output] no_protection" to perform media segment repackaging only. 

Run "packager_proxy" to start the dynamic packager proxy. The proxy listens on port 8080. A docker container tar file is provided with all the setup for testing packager_proxy. 

Known issues and limitations
    - The current version of this library is only intended for repackaging media segments, not repackaging/segmenting large video files into HLS/DASH streams. For instance, when integrated into an OTT streaming server, dynaMediaPackager reads the mezzanine (intermediate) media segments, and dynamically repackages into container (e.g. ts, fmp4) and encryption (e.g. cenc, cens, cbcs, cbc1) formats that are requested by streaming players. Please refer to the sample packager_proxy application for details.  

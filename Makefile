CXX = g++

CC = gcc

APP = packager_proxy

INCLUDE = -I../shaka_packager/src/ -I../shaka_packager/src/packager/

SRC = packagerMain.cc segment_utils.cc getSegmentTimestamp.cc setSegmentTimestamp.cc extractInitAndMediaSegment.cc

OBJ = packagerMain.o segment_utils.o getSegmentTimestamp.o setSegmentTimestamp.o extractInitAndMediaSegment.o

SHAKA_LIB_BASE = ../shaka_packager/src/out/Release/

CPPFLAGS = -std=c++11 -DNDEBUG

LIBS_PATH = -L./ -L$(SHAKA_LIB_BASE)/ -L$(SHAKA_LIB_BASE)/obj/base/ -L$(SHAKA_LIB_BASE)/obj/third_party/ -L$(SHAKA_LIB_BASE)/obj/file/ -L$(SHAKA_LIB_BASE)/obj/base/third_party/libevent/ -L$(SHAKA_LIB_BASE)/obj/base/third_party/dynamic_annotations/ -L$(SHAKA_LIB_BASE)/obj/ -L$(SHAKA_LIB_BASE)/obj/hls/ -L$(SHAKA_LIB_BASE)/obj/media/ -L$(SHAKA_LIB_BASE)/obj/media/base/ -L$(SHAKA_LIB_BASE)/obj/third_party/protobuf/ -L$(SHAKA_LIB_BASE)/obj/third_party/libxml/ -L$(SHAKA_LIB_BASE)/obj/third_party/zlib/ -L$(SHAKA_LIB_BASE)/obj/mpd/ -L$(SHAKA_LIB_BASE)/obj/media/chunking/ -L$(SHAKA_LIB_BASE)/obj/media/codecs/ -L$(SHAKA_LIB_BASE)/obj/media/crypto/ -L$(SHAKA_LIB_BASE)/obj/media/demuxer/ -L$(SHAKA_LIB_BASE)/obj/media/formats/ -L$(SHAKA_LIB_BASE)/obj/third_party/libpng/ -L$(SHAKA_LIB_BASE)/obj/media/formats/mp4/ -L$(SHAKA_LIB_BASE)/obj/media/event/ -L$(SHAKA_LIB_BASE)/obj/media/origin/ -L$(SHAKA_LIB_BASE)/obj/media/formats/packed_audio/ -L$(SHAKA_LIB_BASE)/obj/media/formats/mp2t/ -L$(SHAKA_LIB_BASE)/obj/media/formats/dvb/ -L$(SHAKA_LIB_BASE)/obj/media/formats/ttml/ -L$(SHAKA_LIB_BASE)/obj/third_party/zlib/ -L$(SHAKA_LIB_BASE)/obj/third_party/boringssl/ -L$(SHAKA_LIB_BASE)/obj/third_party/icu/ -L$(SHAKA_LIB_BASE)/obj/third_party/modp_b64/ -L$(SHAKA_LIB_BASE)/obj/third_party/gflags/ -L$(SHAKA_LIB_BASE)/obj/version/ -L$(SHAKA_LIB_BASE)/obj/third_party/curl/ -L$(SHAKA_LIB_BASE)/obj/media/formats/webm/ -L$(SHAKA_LIB_BASE)/obj/third_party/libwebm/ -L$(SHAKA_LIB_BASE)/obj/media/event/ -L$(SHAKA_LIB_BASE)/obj/media/formats/ -L$(SHAKA_LIB_BASE)/obj/media/formats/webvtt/ -L$(SHAKA_LIB_BASE)/obj/media/trick_play/ -L$(SHAKA_LIB_BASE)/obj/media/formats/wvm/ -L$(SHAKA_LIB_BASE)/obj/media/replicator/ -L/usr/lib/x86_64-linux-gnu/

LIBS = -lpackager -lcrypto -lttml -lmpd_builder -lhls_builder -lfile -levent -lwidevine_pssh_data_proto -lstatus -lpacked_audio -ltrick_play -lorigin -ldemuxer -lmp4 -lxml2 -licuuc -lchrome_zlib -lzlib_x86_simd -lgflags -lmp2t -lwebm -lmedia_event -lcodecs -ldvb -lpng -lmedia_base -lbase -lbase_static -lmodp_b64 -lwidevine_common_encryption_proto -levent -lversion -lsymbolize -lmanifest_base -lwebvtt -lwvm -lreplicator -lchunking -lmedia_info_proto -lprotobuf_full_do_not_use -lmkvmuxer -lcurl -lboringssl -lboringssl_asm -lrt -ldl -latomic -lm -lpthread -lstdc++ -lcares

all: dynaMediaPackager $(APP)

packaging_utils.o mediaPackager.o libMediaPackager.o: packaging_utils.cc mediaPackager.cc libMediaPackager.cc
	$(CXX) -g -O -c $(CPPFLAGS) packaging_utils.cc mediaPackager.cc libMediaPackager.cc $(INCLUDE) 

libMediaPackager.a: libMediaPackager.o packaging_utils.o mediaPackager.o
	ar rcs libMediaPackager.a libMediaPackager.o packaging_utils.o mediaPackager.o

dynaMediaPackager: libMediaPackager.a 
	$(CC) -g -O packagerMain.c $(INCLUDE) $(LIBS_PATH) -lMediaPackager $(LIBS) -o dynaMediaPackager

$(APP): libMediaPackager.a
	go build packager_proxy.go
	#CGO_LDFLAGS_ALLOW='-Wl,-no_compact_unwind' CGO_LDFLAGS='-L./ -L/usr/local/lib/ -L/usr/local/opt/bzip2/lib/ -L/usr/local/opt/libiconv/lib/ -L/usr/lib/x86_64-linux-gnu/ -lc++ -lz -lm -lbz2 -liconv.2.4.0 -lMediaPackager' go build packager_proxy.go
	#CGO_LDFLAGS='-L./ -L/usr/lib/x86_64-linux-gnu -lMediaPackager' go build packager_proxy.go

clean:
	-@rm packager_proxy dynaMediaPackager libMediaPackager.a *.o 2> /dev/null || true

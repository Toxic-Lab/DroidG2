LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION	:= cpp
LOCAL_MODULE    := sharelin

LOCAL_SRC_FILES := src/main.cpp \
src/sl.cpp \
src/arrivalcache.cpp \
src/asynccopy.cpp \
src/asyncfile.cpp \
src/asyncfunction.cpp \
src/asynchash.cpp \
src/asynctask.cpp \
src/asynctaskserver.cpp \
src/buffer.cpp \
src/contenttype.cpp \
src/departurecache.cpp \
src/discovery.cpp \
src/download.cpp \
src/downloader.cpp \
src/downloader_impl.cpp \
src/downloadmanager.cpp \
src/env.cpp \
src/filemanager.cpp \
src/filemap.cpp \
src/filesystem.cpp \
src/g2handshake.cpp \
src/g2misc.cpp \
src/g2net.cpp \
src/g2node.cpp \
src/g2nodeinfo.cpp \
src/gatekeeper.cpp \
src/guid.cpp \
src/hashes.cpp \
src/history.cpp \
src/hubcache.cpp \
src/idgen.cpp \
src/ip.cpp \
src/logger.cpp \
src/managedinfo.cpp \
src/media.cpp \
src/period.cpp \
src/queryhashtable.cpp \
src/searcher.cpp \
src/searchmanager.cpp \
src/security.cpp \
src/settings.cpp \
src/sharecache.cpp \
src/sharedfile.cpp \
src/sharemanager.cpp \
src/sharesync.cpp \
src/sourcehost.cpp \
src/staticname.cpp \
src/system.cpp \
src/tigertree.cpp \
src/trafmanager.cpp \
src/transceiver.cpp \
src/udp.cpp \
src/uicommons.cpp \
src/uicomparers.cpp \
src/updateswatch.cpp \
src/uploader_impl.cpp \
src/uploadmanager.cpp \
src/url.cpp \
src/webuiclient.cpp \
src/webuicontrol.cpp \
src/webuiserver.cpp \
src/zlib.cpp \
src/conv/conv.cpp \
src/g2packet/packet.cpp \
src/g2packet/reader.cpp \
src/g2packet/util.cpp \
src/g2packet/writer.cpp \
src/hash/hash.cpp \
src/hash/ed2khash.c \
src/hash/md5.c \
src/hash/sboxes.c \
src/hash/sha1.c \
src/hash/tiger.c \
src/hash/ttr.c \
src/ui/command.cpp \
src/ui/template.cpp boob

LOCAL_CFLAGS += -g -DMYSTIC -I$(LOCAL_PATH)/boost/include/
LOCAL_LDLIBS := \
-lm -llog \
-lz \
-L$(LOCAL_PATH)/boost/lib


LOCAL_STATIC_LIBRARIES := \
    libboost_filesystem-mt-1_45 \
    libboost_system-mt-1_45 \
    libboost_thread-mt-1_45
    
LOCAL_CPPFLAGS += -DMYSTIC -I$(LOCAL_PATH)/boost/include/ 
LOCAL_CPPFLAGS += -fexceptions
LOCAL_CPPFLAGS += -frtti
LOCAL_CPPFLAGS +=-DBOOST_THREAD_LINUX
LOCAL_CPPFLAGS +=-DBOOST_HAS_PTHREADS
LOCAL_CPPFLAGS +=-D__arm__
LOCAL_CPPFLAGS +=-D_REENTRANT
LOCAL_CPPFLAGS +=-D_GLIBCXX__PTHREADS
LOCAL_CPPFLAGS +=-DBOOST_HAS_GETTIMEOFDAY

include $(BUILD_SHARED_LIBRARY)

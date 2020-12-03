TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH +=  /usr/local/include \
                /usr/local/include/opencv \
                /usr/local/include/opencv2 \
                /usr/local/ffmpeg/include

LIBS+= /usr/local/lib/libopencv_*.so

LIBS += -L/usr/local/ffmpeg/lib
LIBS += -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale


SOURCES += main.cpp \
    logku/logger.cpp \
    gate.cpp \
    base/videoplayer.cpp \
    base/jsonpara.cpp

HEADERS += \
    third/nlohmannjson.h \
    base/ucfunc.h \
    base/uconfig.h \
    logku/logger.h \
    base/videoplayer.h \
    base/jsonpara.h

LIBS += -lpthread


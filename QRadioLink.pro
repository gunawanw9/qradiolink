#-------------------------------------------------
#
# Project created by QtCreator 2013-10-25T13:18:22
#
#-------------------------------------------------

QT       += core network sql

QT       -= gui

TARGET = qradiolink
CONFIG   += console
CONFIG   -= app_bundle
CONFIG  += thread

TEMPLATE = app


SOURCES += main.cpp \
    speech.cpp \
    audiointerface.cpp \
    dtmfdecoder.cpp \
    telnetclient.cpp \
    telnetserver.cpp \
    serverwrapper.cpp \
    dtmfcommand.cpp \
    databaseapi.cpp \
    controller.cpp \
    station.cpp \
    server.cpp \
    ext/Mumble.pb.cc \
    ext/QRadioLink.pb.cc \
    audioencoder.cpp \
    mumbleclient.cpp \
    sslclient.cpp \
    ext/utils.cpp \
    audioop.cpp \
    ext/goertzel.cpp \
    ext/agc.cpp \
    ext/vox.cpp \
    settings.cpp
#    videoop.cpp \
#    videoencoder.cpp


HEADERS += speech.h \
    audiointerface.h \
    dtmfdecoder.h \
    telnetclient.h \
    telnetserver.h \
    serverwrapper.h \
    dtmfcommand.h \
    databaseapi.h \
    controller.h \
    station.h \
    server.h \
    config_defines.h \
    ext/Mumble.pb.h \
    ext/murmur_pch.h \
    ext/PacketDataStream.h \
    ext/QRadioLink.pb.h \
    audioencoder.h \
    mumbleclient.h \
    sslclient.h \
    ext/utils.h \
    codec2/codec2.h \
    audioop.h \
    ext/goertzel.h \
    ext/Goertzel.h \
    ext/agc.h \
    ext/vox.h \
    ext/dec.h \
    settings.h
#    videoop.h \
#    videoencoder.h




unix:!symbian: LIBS += -lprotobuf -lopus  -lFestival -lpulse-simple -lestools -lestbase -leststring
unix:!symbian: LIBS += -L$$PWD/codec2 -lcodec2
#unix:!symbian: LIBS += -L$$PWD/iaxclient/build -liaxclient_lib

#unix:!symbian: LIBS += -lopencv_highgui -lopencv_core -lopencv_imgproc -ltheora

#DEPENDPATH += $$PWD/iaxclient/build
INCLUDEPATH += /usr/include/speech_tools



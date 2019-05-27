#-------------------------------------------------
#
# Project created by QtCreator 2012-08-03T23:05:23
#
#-------------------------------------------------

QT       += core gui

TARGET = QTGui
TEMPLATE = app


SOURCES +=\
    GUI/mainwindow.cpp \
    Main/main.cpp \
    Main/Factory.cpp \
    AudioPlayer/ALSAWrapper/ALSAWrapper.cpp \
    AudioPlayer/DeviceInfo.cpp \
    AudioPlayer/AudioPlayer.cpp \
    AudioPlayer/AudioFormats/Mp3Decoder.cpp \
    AudioPlayer/FormatRecognizer.cpp \
    AudioPlayer/DecoderBase.cpp \
    AudioPlayer/WaveReader.cpp \
    AudioPlayer/MP3Reader.cpp


HEADERS  += \
    GUI/mainwindow.h \
    AudioPlayer/DeviceInfo.h \
    AudioPlayer/AudioPlayer.h \
    Main/Factory.h \
    AudioPlayer/ALSAWrapper/ALSAWrapper.h \
    AudioPlayer/FormatRecognizer.h \
    AudioPlayer/AudioFormats/DecoderBase.h \
    AudioPlayer/AudioFormats/Mp3Decoder.h \
    AudioPlayer/IFFileFormat.h \
    AudioPlayer/MP3Reader.h \
    AudioPlayer/WaveReader.h

FORMS    += \
    GUI/mainwindow.ui

LIBS += -L/usr/lib/x86_64-linux-gnu/  -lasound -lmad

OTHER_FILES += \
    Codes.txt

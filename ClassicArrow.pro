#-------------------------------------------------
#
# Project created by QtCreator 2017-01-30T14:19:11
#
#-------------------------------------------------

QT       -= gui

TARGET = ClassicArrow
TEMPLATE = lib
CONFIG += dll

DEFINES += CLASSICARROW_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
	ccallhook.cpp \
	memsafe.cpp \
    cshortasm.cpp \
    sys/mman.c \
	main.cpp

HEADERS +=\
    sys/mman.h \
	ccallhook.h \
	memsafe.h \
	cshortasm.h

LIBS += -luser32

QMAKE_LFLAGS_RELEASE += -static -static-libgcc -static-libstdc++

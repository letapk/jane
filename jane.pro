#Last modified 5 Sep 2026

QT += core gui widgets

#This should be uncommented to produce a compile time error
#for string literals that are not enclosed within a tr()
#DEFINES  += QT_NO_CAST_FROM_ASCII

TARGET = jane
TEMPLATE = app

SOURCES +=\
    janefile.cpp \
    janelist.cpp \
    janemain.cpp \
    janemisc.cpp \
    janeprefs.cpp

HEADERS	= jane.h

OTHER_FILES += \
    TODO \
    README \
    COPYING \
    AUTHORS

RESOURCES += \
    jane.qrc

LIBS += -lqtspell-qt6


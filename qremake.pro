CONFIG += c++14
QT = core core-private

DEFINES += \
    QT_NO_CAST_FROM_BYTEARRAY \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII

SOURCES += \
    main.cpp \
    qscheme.cpp

HEADERS += \
    qscheme.h \
    qtschemeglobal.h

RESOURCES += \
    resources.qrc

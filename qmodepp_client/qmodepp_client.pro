QT += network
TEMPLATE = vcapp
TARGET = qmodepp_client
DESTDIR = bini
DEFINES += MODEPP_INCLUDE_MESSAGE_TYPES_ONLY
INCLUDEPATH +=  ../modepp_server
RESOURCES += src.qrc 

SOURCES += main.cpp mainwindow.cpp about.cpp
HEADERS += mainwindow.h  about.h
FORMS += mainwindow.ui about.ui

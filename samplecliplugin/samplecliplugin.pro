include(../plugins.pri)

TARGET = samplecliplugin
OTHER_FILES += samplecliplugin/.json

HEADERS += src/samplecliplugin.h

SOURCES += src/samplecliplugin.cpp

target.path = $$PLUGINSDIR/$$TARGET

INSTALLS += target

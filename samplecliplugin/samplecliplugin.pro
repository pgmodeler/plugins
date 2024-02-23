include(../plugins.pri)

TARGET = samplecliplugin
TRANSLATIONS += $$PWD/lang/en_US.ts

HEADERS += src/samplecliplugin.h

SOURCES += src/samplecliplugin.cpp

target.path = $$PLUGINSDIR/$$TARGET
resources.path = $$PLUGINSDIR/$$TARGET
resources.files += lang

INSTALLS += target resources

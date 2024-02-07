include(../plugins.pri)

TARGET = sampleplugin
TRANSLATIONS += $$PWD/lang/$$TARGET.en_US.ts

RESOURCES += res/sampleplugin.qrc

HEADERS += src/sampleplugin.h

SOURCES += src/sampleplugin.cpp

target.path = $$PLUGINSDIR/$$TARGET
resources.path = $$PLUGINSDIR/$$TARGET
resources.files += lang

INSTALLS += target resources

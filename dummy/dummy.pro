include(../plugins.pri)

TARGET = dummy
OTHER_FILES += dummy.json
TRANSLATIONS += $$PWD/lang/$$TARGET.en_US.ts
HEADERS += src/dummy.h
SOURCES += src/dummy.cpp

target.path = $$PLUGINSDIR/$$TARGET
resources.path = $$PLUGINSDIR/$$TARGET
resources.files += res/dummy.png lang dummy.json

INSTALLS += target resources

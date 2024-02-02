include(../plugins.pri)

PGMODELERCLI_SRC = $$absolute_path($$PGMODELER_SRC/apps/pgmodeler-cli/src)

TARGET = samplecliplugin
OTHER_FILES += samplecliplugin.json
TRANSLATIONS += $$PWD/lang/$$TARGET.en_US.ts

INCLUDEPATH += $$PGMODELERCLI_SRC

HEADERS += src/samplecliplugin.h

SOURCES += src/samplecliplugin.cpp

target.path = $$PLUGINSDIR/$$TARGET
resources.path = $$PLUGINSDIR/$$TARGET
resources.files += lang sampleplugin.json

INSTALLS += target resources

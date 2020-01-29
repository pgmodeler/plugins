include(../plugins.pri)

TARGET = xml2object
OTHER_FILES += xml2object.json
TRANSLATIONS += $$PWD/lang/$$TARGET.en_US.ts

HEADERS += src/xml2object.h \
		   src/xml2objectwidget.h

SOURCES += src/xml2object.cpp \
		   src/xml2objectwidget.cpp

FORMS += ui/xml2objectwidget.ui

target.path = $$PLUGINSDIR/$$TARGET
resources.path = $$PLUGINSDIR/$$TARGET
resources.files += res/xml2object.png lang xml2object.json

INSTALLS += target resources

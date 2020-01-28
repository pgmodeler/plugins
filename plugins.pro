# plugins.pro (original version)
#
# Original version by: Lisandro Damián Nicanor Pérez Meyer <perezmeyer@gmail.com>
# Original code: https://github.com/perezmeyer/pgmodeler/tree/shared_libs

include(../pgmodeler.pri)

macx {
 QMAKE_POST_LINK += $$OUT_PWD/../patch_plugin_dylib.sh lib$${TARGET}.$${QMAKE_EXTENSION_SHLIB}
}

SUBDIRS += dummy xml2object graphicalquerybuilder



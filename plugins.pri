include(../pgmodeler.pri)

CONFIG += plugin qt
QT += core gui uitools
TEMPLATE = lib
DEPENDPATH = ". res src ui moc obj"
MOC_DIR = moc
OBJECTS_DIR = obj
UI_DIR = src

windows: DESTDIR += $$PWD

unix|windows: LIBS += -L$$OUT_PWD/../../libpgmodeler_ui/ -lpgmodeler_ui \
                      -L$$OUT_PWD/../../libobjrenderer/ -lobjrenderer \
                      -L$$OUT_PWD/../../libpgconnector/ -lpgconnector \
                      -L$$OUT_PWD/../../libpgmodeler/ -lpgmodeler \
                      -L$$OUT_PWD/../../libparsers/ -lparsers \
                      -L$$OUT_PWD/../../libutils/ -lutils

INCLUDEPATH += $$PWD/../libpgmodeler_ui \
               $$PWD/../libpgmodeler_ui/src \
               $$PWD/../libobjrenderer/src \
               $$PWD/../libpgconnector/src \
               $$PWD/../libpgmodeler/src \
               $$PWD/../libparsers/src \
               $$PWD/../libutils/src

DEPENDPATH += $$PWD/../libpgmodeler_ui \
              $$PWD/../libobjrenderer \
              $$PWD/../libpgconnector \
              $$PWD/../libpgmodeler \
              $$PWD/../libparsers \
              $$PWD/../libutils

macx {
  OUTPUT_PLUGIN = $$PWD/$${TARGET}/lib$${TARGET}.dylib
  OLD_FW_REF = @loader_path/../Frameworks
  NEW_FW_REF = @loader_path/../../../Frameworks

  QMAKE_POST_LINK += install_name_tool -change $$OLD_FW_REF/libutils.1.dylib $$NEW_FW_REF/libutils.1.dylib $$OUTPUT_PLUGIN ; \
                     install_name_tool -change $$OLD_FW_REF/libparsers.1.dylib $$NEW_FW_REF/libparsers.1.dylib $$OUTPUT_PLUGIN ; \
                     install_name_tool -change $$OLD_FW_REF/libpgmodeler.1.dylib $$NEW_FW_REF/libpgmodeler.1.dylib $$OUTPUT_PLUGIN ; \
                     install_name_tool -change $$OLD_FW_REF/libpgmodeler_ui.1.dylib $$NEW_FW_REF/libpgmodeler_ui.1.dylib $$OUTPUT_PLUGIN ; \
                     install_name_tool -change $$OLD_FW_REF/libpgconnector.1.dylib $$NEW_FW_REF/libpgconnector.1.dylib $$OUTPUT_PLUGIN ; \
                     install_name_tool -change $$OLD_FW_REF/libobjrenderer.1.dylib $$NEW_FW_REF/libobjrenderer.1.dylib $$OUTPUT_PLUGIN
}

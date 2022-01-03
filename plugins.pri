include(../pgmodeler.pri)

CONFIG += plugin qt
QT += core gui uitools
TEMPLATE = lib
DEPENDPATH = ". res src ui moc obj"
MOC_DIR = moc
OBJECTS_DIR = obj
UI_DIR = src

windows: DESTDIR += $$PWD

unix|windows: LIBS += $$LIBGUI_LIB \
		      $$LIBCANVAS_LIB \
		      $$LIBCONNECTOR_LIB \
		      $$LIBCORE_LIB \
		      $$LIBPARSERS_LIB \
		      $$LIBUTILS_LIB

INCLUDEPATH += $$LIBGUI_INC \
	       $$LIBCANVAS_INC \
	       $$LIBCONNECTOR_INC \
	       $$LIBCORE_INC \
	       $$LIBPARSERS_INC \
	       $$LIBUTILS_INC

DEPENDPATH += $$LIBGUI_ROOT \
	      $$LIBCANVAS_ROOT \
	      $$LIBCONNECTOR_ROOT \
	      $$LIBCORE_ROOT \
	      $$LIBPARSERS_ROOT \
	      $$LIBUTILS_ROOT

macx {
  OUTPUT_PLUGIN = $$PWD/$${TARGET}/lib$${TARGET}.dylib
  OLD_FW_REF = @loader_path/../Frameworks
  NEW_FW_REF = @loader_path/../../../Frameworks

  QMAKE_POST_LINK += install_name_tool -change $$OLD_FW_REF/$$LIBUTILS.1.dylib $$NEW_FW_REF/$$LIBUTILS.1.dylib $$OUTPUT_PLUGIN ; \
		     install_name_tool -change $$OLD_FW_REF/$$LIBPARSERS.1.dylib $$NEW_FW_REF/$$LIBPARSERS.1.dylib $$OUTPUT_PLUGIN ; \
		     install_name_tool -change $$OLD_FW_REF/$$LIBCORE.1.dylib $$NEW_FW_REF/$$LIBCORE.1.dylib $$OUTPUT_PLUGIN ; \
		     install_name_tool -change $$OLD_FW_REF/$$LIBGUI.1.dylib $$NEW_FW_REF/$$LIBGUI.1.dylib $$OUTPUT_PLUGIN ; \
		     install_name_tool -change $$OLD_FW_REF/$$LIBCONNECTOR.1.dylib $$NEW_FW_REF/$$LIBCONNECTOR.1.dylib $$OUTPUT_PLUGIN ; \
		     install_name_tool -change $$OLD_FW_REF/$$LIBCANVAS.1.dylib $$NEW_FW_REF/$$LIBCANVAS.1.dylib $$OUTPUT_PLUGIN
}

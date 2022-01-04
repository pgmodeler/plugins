# This file contains the main variables settings to build pgModeler plugins on all supported platforms.
# Plugins can be built either together with pgModeler or in standalone mode by using a previous installed pgModeler.
# To build together with pgModeler nothing has to be done since all needed values are already defined.

# To build in standalone mode three variables must be provided to qmake prior the building
# so de default values can be replaced by custom paths:
#
# PGMODELER_PLUGINS -> Full path to where the pgModeler plugins are installed (providing this variable automatically switches to standalone building)
# PGMODELER_SRC     -> Full path to pgModeler source code
# PGMODELER_LIBS    -> Full path to pgModeler libraries

# If the plugins custom prefix (standalone build) is defined we check if the provided source code path is valid
defined(PGMODELER_PLUGINS, var):{
    !defined(PGMODELER_SRC, var):error("A valid path to the pgModeler source code must be provided using PGMODELER_SRC=[path]! Aborting.")
    !exists($$PGMODELER_SRC/pgmodeler.pri):error("The provided source code path $$PGMODELER_SRC doesn't seem to be pgModeler's source code! Aborting.")
}

# If no custom path to pgModeler's source code is provided we assume that the plugin is being build together with pgModeler
!defined(PGMODELER_PLUGINS, var):PGMODELER_SRC=../

include($$PGMODELER_SRC/pgmodeler.pri)

CONFIG += plugin qt
QT += core gui uitools
TEMPLATE = lib
DEPENDPATH = ". res src ui moc obj"
MOC_DIR = moc
OBJECTS_DIR = obj
UI_DIR = src
windows: DESTDIR += $$PWD

# Only for standalone building: we need check and replace some variables so the built plugin can reference external libs and source
defined(PGMODELER_PLUGINS, var):{
    # Rewritting the output path for plugins so they can be installed in the custom prefix provided by the user
    PLUGINSDIR=$$PGMODELER_PLUGINS

    macx:LIB_EXT=dylib
    !macx|!windows:LIB_EXT=so
    windows:LIB_EXT=dll
	unix:LIB_SUFFIX=lib

    !defined(PGMODELER_LIBS, var):error("A valid path to the pgModeler libraries must be provided using PGMODELER_LIBS=[path]! Aborting.")
    !exists($$PGMODELER_LIBS/$${LIB_SUFFIX}core.$${LIB_EXT}):error("The provided libraries path $$PGMODELER_LIBS doesn't seem to store pgModeler's libraries! Aborting.")

    # Configuring the subproject sources path according to the custom source code path
    LIBS_SRC=$$PGMODELER_SRC/libs/
    LIBGUI_INC=$$LIBS_SRC/$$LIBGUI/src
    LIBCANVAS_INC=$$LIBS_SRC/$$LIBCANVAS/src
    LIBCONNECTOR_INC=$$LIBS_SRC/$$LIBCONNECTOR/src
    LIBCORE_INC=$$LIBS_SRC/$$LIBCORE/src
    LIBPARSERS_INC=$$LIBS_SRC/$$LIBPARSERS/src
    LIBUTILS_INC=$$LIBS_SRC/$$LIBUTILS/src

    # Adding the custom path to pgModeler libraries as a search path for linker
    LIBS+=-L$$PGMODELER_LIBS
}

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

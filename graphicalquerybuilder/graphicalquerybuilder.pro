# graphicalquerybuilder.pro 
#
# Adapted by: Maxime Chambonnet <maxzor@maxzor.eu> - 01/01/2019
# Refactored by: Lisandro Damián Nicanor Pérez Meyer <perezmeyer@gmail.com>
# Refactored code: https://github.com/perezmeyer/pgmodeler/tree/shared_libs
# Reviewed by: Raphal Araújo e Silva <raphael@pgmodeler.com.br>
#
# NOTE: Reviewed code is not a direct merge from refactored version but based upon the
# refactored code, containing almost all changes done by the refactoring author.

# Unix or Windows directory configuration
PGMODELER_SRC_DIR=../../

!exists($$PGMODELER_SRC_DIR) {
 warning("The pgModeler source code directory '$$PGMODELER_SRC_DIR' could not be found! Make sure the variable PGMODELER_SRC_DIR points to a valid location!")
 error("qmake aborted.")
}

include(../../pgmodeler.pri)

CONFIG *= plugin qt uic4
QT *= core gui uitools
TEMPLATE = lib
TARGET = graphicalquerybuilder
TRANSLATIONS += $$PWD/lang/$$TARGET.en_US.ts
OTHER_FILES += graphicalquerybuilder.json
CODECFORTR = UTF8
DEPENDPATH = ". res src ui moc obj"
MOC_DIR = moc
OBJECTS_DIR = obj
UI_DIR = src

HEADERS += src/graphicalquerybuilder.h \
			src/graphicalquerybuildercorewidget.h \
			src/graphicalquerybuilderpathwidget.h \
			src/graphicalquerybuildersqlwidget.h

SOURCES += src/graphicalquerybuilder.cpp \
			src/graphicalquerybuildercorewidget.cpp \
			src/graphicalquerybuilderpathwidget.cpp \
			src/graphicalquerybuildersqlwidget.cpp

FORMS += ui/graphicalquerybuildercorewidget.ui \
			ui/graphicalquerybuilderpathwidget.ui \
			ui/graphicalquerybuildersqlwidget.ui

windows: DESTDIR += $$PWD

unix|windows: LIBS += -L$$OUT_PWD/../../libpgmodeler_ui/ -lpgmodeler_ui \
					  -L$$OUT_PWD/../../libobjrenderer/ -lobjrenderer \
					  -L$$OUT_PWD/../../libpgconnector/ -lpgconnector \
					  -L$$OUT_PWD/../../libpgmodeler/ -lpgmodeler \
					  -L$$OUT_PWD/../../libparsers/ -lparsers \
					  -L$$OUT_PWD/../../libutils/ -lutils

INCLUDEPATH += $$PWD/../../libpgmodeler_ui \
			   $$PWD/../../libpgmodeler_ui/src \
			   $$PWD/../../libobjrenderer/src \
			   $$PWD/../../libpgconnector/src \
			   $$PWD/../../libpgmodeler/src \
			   $$PWD/../../libparsers/src \
			   $$PWD/../../libutils/src \


DEPENDPATH += $$PWD/../../libpgmodeler_ui \
			  $$PWD/../../libobjrenderer \
			  $$PWD/../../libpgconnector \
			  $$PWD/../../libpgmodeler \
			  $$PWD/../../libparsers \
			  $$PWD/../../libutils \

# Will build the graphical-query-builder join solver,
# if the variable is written in the conf file.
GQB_JOIN_SOLVER=$$fromfile($$PWD/../plugins.conf, GQB_JOIN_SOLVER)
equals(GQB_JOIN_SOLVER,"y"){

	GQBS_WARNING_END=""
	!exists(paal){
		GQBS_WARNING_END = "Paal files were not found at $$PWD/paal!"
	}
	exists(paal){
		!exists(paal/boost/boost){
			GQBS_WARNING_END = "Boost headers were not found at $$PWD/paal/boost/boost !"
		}
	}

	!equals(GQBS_WARNING_END, ""){

		warning("Graphical query builder : To build the join solver,")
		warning("make sure to run the setup script, or to configure the plugin dependencies (paal & boost) correctly !")
		warning($$GQBS_WARNING_END)
		warning("I am dumb, be advised before next question, and you would be in for a ride answering yes.")
		answer=$$prompt("Do you want me to run the setup script now (and download a few hundred MBs) Y/n ?")

		!equals(answer, "n"){
			equals(GQBS_WARNING_END,"Paal files were not found at $$PWD/paal!"){
				system("./setup.sh")
			}
			equals(GQBS_WARNING_END,"Boost headers were not found at $$PWD/paal/boost/boost !"){
				system("./setup.sh | tail -13")
			}
		}
		equals(answer, "n"){
			error("PgModeler compilation aborted : the SQL-join solver of the graphical-query-builder plugin errorred out.")
		}
	}

	#message("GQB join solver!")
	DEFINES += "GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER"
	HEADERS += src/graphicalquerybuilderjoinsolver.h
	SOURCES += src/graphicalquerybuilderjoinsolver.cpp
	INCLUDEPATH += $$PWD/paal/include \
				   $$PWD/paal/boost 
	DEPENDPATH += $$PWD/paal/include \
				   $$PWD/paal/boost
}


target.path = $$PLUGINSDIR/$$TARGET

resources.path = $$PLUGINSDIR/$$TARGET
resources.files += res/graphicalquerybuilder.png lang graphicalquerybuilder.json

INSTALLS += target resources

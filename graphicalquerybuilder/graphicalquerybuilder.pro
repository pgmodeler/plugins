# graphicalquerybuilder.pro 
#
# Adapted by: Maxime Chambonnet <maxzor@maxzor.eu> - 01/05/2019
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

include(../plugins.pro)

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

#------------------------------------------------------------------------------------------------
LB=$$escape_expand(\n)
T=$$escape_expand(\t)

INTERACTIVE_QMAKE{
	answer=$$prompt(" * Do you want to compile the SQL-join solver of the graphical query builder plugin ? [y/N]", false)
	equals(answer, "y"){
		system("sed -i.bak s/GQB_JOIN_SOLVER=\\\"n\\\"/GQB_JOIN_SOLVER=\\\"y\\\"/ $$PWD/graphicalquerybuilder.conf")
	}
	else{
		system("sed -i.bak s/GQB_JOIN_SOLVER=\\\"y\\\"/GQB_JOIN_SOLVER=\\\"n\\\"/ $$PWD/graphicalquerybuilder.conf")
	}
}

GQB_JOIN_SOLVER=$$fromfile($$PWD/graphicalquerybuilder.conf, GQB_JOIN_SOLVER)
BOOST_INSTALLED=$$fromfile($$PWD/graphicalquerybuilder.conf, BOOST_INSTALLED)
equals(GQB_JOIN_SOLVER, "y"){

	GQBS_WARNING_END=""
	!exists(paal){
		GQBS_MISSING_DEP= "paal"
		GQBS_WARNING_END = "Paal files were not found at $$LB$$T$$T $$PWD/paal !"
	}
	else:!exists(paal/boost/boost):equals(BOOST_INSTALLED,"n"){
			GQBS_MISSING_DEP="boost"
			GQBS_WARNING_END = "Boost headers were not found at $$LB$$T$$T $$PWD/paal/boost/boost !"
	}

	!equals(GQBS_WARNING_END, ""):INTERACTIVE_QMAKE{

		warning("Graphical-query-builder SQL-join solver dependency issue : ")
		log($$T$$T $$GQBS_WARNING_END $$LB)
		answer=$$prompt("Do you want me to try and setup the dependencies ? [Y/n]")
		equals(answer, "n"){
			error("PgModeler compilation aborted : the SQL-join solver of the graphical-query-builder plugin errorred out.")
		}
		else{
			answer=$$prompt("Do you already have boost installed on your system ? [y/N]")
			!equals(answer, "y"){
				system("sed -i.bak s/BOOST_INSTALLED=\\\"y\\\"/BOOST_INSTALLED=\\\"n\\\"/ $$PWD/graphicalquerybuilder.conf")
				equals(GQBS_MISSING_DEP, "paal"){
					system("./setup.sh paal boost")
				}
				else{
					system("./setup.sh boost")
				}
			}
			equals(answer, "y"){
				system("sed -i.bak s/BOOST_INSTALLED=\\\"n\\\"/BOOST_INSTALLED=\\\"y\\\"/ $$PWD/graphicalquerybuilder.conf")
				equals(GQBS_MISSING_DEP, "paal"){
					system("./setup.sh paal")
				}
			}
		}
	}

	!equals(GQBS_WARNING_END, ""):!INTERACTIVE_QMAKE{
		warning($$GQBS_WARNING_END)
		error("PgModeler compilation aborted : the SQL-join solver of the graphical-query-builder plugin errorred out.")
	}

	#message("GQB join solver!")
	DEFINES += "GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER"
	HEADERS += src/graphicalquerybuilderjoinsolver.h
	SOURCES += src/graphicalquerybuilderjoinsolver.cpp
	INCLUDEPATH += $$PWD/paal/include
				   $$PWD/paal/boost
	DEPENDPATH += $$PWD/paal/include
				   $$PWD/paal/boost
}
#------------------------------------------------------------------------------------------------

target.path = $$PLUGINSDIR/$$TARGET

resources.path = $$PLUGINSDIR/$$TARGET
resources.files += res/graphicalquerybuilder.png lang graphicalquerybuilder.json

INSTALLS += target resources

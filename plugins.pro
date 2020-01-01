# plugins.pro
#
# Original version by : Maxime Chambonnet <maxzor@maxzor.eu> - 01/01/2019
include(../pgmodeler.pri)

# NON_INTERACTIVE_QMAKE allows for QtCreator to not block on these prompts.
# https://stackoverflow.com/questions/59553288/how-do-you-check-if-you-are-in-an-interactive-qmake-session-i-want-to-use-qmak
!NON_INTERACTIVE_QMAKE{
	message("Plugin setup, choose what you want to compile :")

	answer=$$prompt("Dummy ? y/N")
	equals(answer,"y"):{
		SUBDIRS += dummy
	}

	answer=$$prompt("Xml2object ? Y/n")
	!equals(answer,"n"){
		SUBDIRS += xml2object
	}

	answer=$$prompt("Graphical query builder core ? Y/n")
	!equals(answer,"n"){
		SUBDIRS += graphicalquerybuilder
		answer=$$prompt("Graphical query builder join solver ? y/N")
		equals(answer, "y"){
			# Currently the only way to pass variables through subdirs
			# This variable will trigger preprocessor macro
			# and includes in graphicalquerybuilder.pro
			system("echo GQB_JOIN_SOLVER=\\\"y\\\" > $$PWD/plugins.conf")
			#write_file($$PWD/plugins.conf, $$GQB_JOIN_SOLVER)
		}
		else {
			system("echo \\\"\\\" > $$PWD/plugins.conf")
			message(GQB conf reset)
		}
	}
}

# Add in QtCreator project tab, in "qmake - details" the flag "CONFIG += NON_INTERACTIVE_QMAKE",
# so that the parser finds the files vv.
NON_INTERACTIVE_QMAKE{
	SUBDIRS += dummy xml2object graphicalquerybuilder
}

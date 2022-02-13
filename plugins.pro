include(plugins.pri)

TEMPLATE = subdirs
SUBDIRS += dummy xml2object

# GQB plugin temporarily disabled due to problems in compilation process
SUBDIRS += graphicalquerybuilder

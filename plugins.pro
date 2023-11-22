include(plugins.pri)

TEMPLATE = subdirs
SUBDIRS += sampleplugin

# Graphical Query Builder plugin is temporarily disabled due to problems in compilation process.
# Currently only the graphical query builder portion can be compiled. The path solver code is failing to build.
# Uncomment the line "SUBDIRS+=..." below if you want to try to build and eventually fix the build process.
#
# Building instructions here:
# >> https://github.com/pgmodeler/plugins/blob/develop/graphicalquerybuilder/README.md

# SUBDIRS += graphicalquerybuilder

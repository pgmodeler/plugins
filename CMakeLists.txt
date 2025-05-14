# Importing Qt6 components
# Forcing a version >= 6.4.0. If no suitable version is found the script aborts
find_package(Qt6 6.4.0 REQUIRED COMPONENTS UiTools)

# By default, the AUTOUIC option makes the uic look for the .ui files
# under the same folder as the sources. In this case, we force the uic
# command to look for .ui files under ui/*
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ui)

include_directories(
    ${LIBCANVAS_INC}
    ${LIBCONNECTOR_INC}
    ${LIBCORE_INC}
    ${LIBCLI_INC}
    ${LIBPARSERS_INC}
    ${LIBUTILS_INC}
    src/
    src/dbobjects
    src/settings
    src/tools
    src/utils
    src/widgets)

link_libraries(
    Qt::Core
    Qt::Gui
    Qt::UiTools
    canvas
    cli
    connector
    core
    gui
    parsers
    utils)

add_subdirectory(sampleplugin)
add_subdirectory(samplecliplugin)

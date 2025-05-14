set(TARGET sampleplugin)

qt_add_plugin(${TARGET} SHARED)
qt_add_resources(PLUGIN_RES "res/${TARGET}.qrc")

target_sources(${TARGET} PRIVATE
    src/sampleplugin.cpp src/sampleplugin.h
    ${PLUGIN_RES})

install(TARGETS ${TARGET}
    LIBRARY DESTINATION ${PGM_PLUGINSDIR}/${TARGET}
    FRAMEWORK DESTINATION ${PGM_PLUGINSDIR}/${TARGET}
    RUNTIME DESTINATION  ${PGM_PLUGINSDIR}/${TARGET})

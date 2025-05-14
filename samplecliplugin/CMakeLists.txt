set(TARGET samplecliplugin)

qt_add_plugin(${TARGET} SHARED)

target_sources(${TARGET} PRIVATE
    src/samplecliplugin.cpp src/samplecliplugin.h)

install(TARGETS ${TARGET}
    LIBRARY DESTINATION ${PGM_PLUGINSDIR}/${TARGET}
    FRAMEWORK DESTINATION ${PGM_PLUGINSDIR}/${TARGET}
    RUNTIME DESTINATION ${PGM_PLUGINSDIR}/${TARGET})

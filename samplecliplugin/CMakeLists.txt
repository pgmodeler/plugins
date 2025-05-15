set(TARGET samplecliplugin)

qt_add_plugin(${TARGET} SHARED)

target_sources(${TARGET} PRIVATE
    src/samplecliplugin.cpp src/samplecliplugin.h)

pgm_install_plugin(${TARGET})

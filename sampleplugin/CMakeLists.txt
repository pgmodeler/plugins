set(TARGET sampleplugin)

qt_add_plugin(${TARGET} SHARED)

target_sources(${TARGET} PRIVATE
    src/sampleplugin.cpp src/sampleplugin.h)

pgm_set_plugin_res(${TARGET})
pgm_install_plugin(${TARGET})

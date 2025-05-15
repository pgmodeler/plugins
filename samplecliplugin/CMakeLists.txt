pgm_add_cli_plugin(samplecliplugin OFF)

target_sources(${PGM_TARGET} PRIVATE
    src/samplecliplugin.cpp src/samplecliplugin.h)

pgm_install_plugin(${PGM_TARGET})

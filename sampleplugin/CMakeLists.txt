pgm_add_cli_plugin(sampleplugin OFF)

target_sources(${PGM_TARGET} PRIVATE
    src/sampleplugin.cpp src/sampleplugin.h)

pgm_set_plugin_res(${PGM_TARGET})
pgm_install_plugin(${PGM_TARGET})

#/bin/bash
install_name_tool -change @loader_path/../Frameworks/libutils.1.dylib @loader_path/../../../Frameworks/libutils.1.dylib $1
install_name_tool -change @loader_path/../Frameworks/libparsers.1.dylib @loader_path/../../../Frameworks/libparsers.1.dylib $1
install_name_tool -change @loader_path/../Frameworks/libpgmodeler.1.dylib @loader_path/../../../Frameworks/libpgmodeler.1.dylib $1
install_name_tool -change @loader_path/../Frameworks/libpgmodeler_ui.1.dylib @loader_path/../../../Frameworks/libpgmodeler_ui.1.dylib $1
install_name_tool -change @loader_path/../Frameworks/libpgconnector.1.dylib @loader_path/../../../Frameworks/libpgconnector.1.dylib $1
install_name_tool -change @loader_path/../Frameworks/libobjrenderer.1.dylib @loader_path/../../../Frameworks/libobjrenderer.1.dylib $1

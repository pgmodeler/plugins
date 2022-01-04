Introduction
------------

This is the official repository of pgModeler plug-ins, it intends to keep a library of extensions created by third-party contributions.

Building plug-ins
-----------------

pgModeler plug-ins can be build in two different ways:

1) Together with a pgModeler building process which generates the entire tool including its plugins. This compilation mode doesn't require any special procedure and the complete steps for the tool's building are available at [project's official web site](https://www.pgmodeler.io/support/installation).

2) Standalone building, which means, compile a plug-ins against an existing pgModeler installation. This compilation mode requires the use of the pgModeler's source code and its shared libraries located at the installation folder.

Standalone build
----------------

In this procedure we assume that you already have the Qt framework installed, a clean installation of pgModeler with unrestricted access to its libraries (.so, .dylib or .dll). So, in order to build a single plug-in and make it recognizable by pgModeler you'll need first to download the tool's source code. Just make sure to download the source related to the same version as your pgModeler installation or the building process may fail. Second, clone this repository anywhere in your system since it's from there that we'll run the steps to achieve our goal. 

For the steps bellow, as a convenience, we have aliased some paths in order to shorten the commands executed, make sure to replace them by the real paths according to your system.

1. Cloning the source code. Alternatively, you can download a [tarball or zip package](https://github.com/pgmodeler/pgmodeler/tags) and extract it anywhere:

    ```
    git clone https://github.com/pgmodeler/pgmodeler.git
    ```
    
    a) Checkout the tag related to the pgModeler installed on your system. Supposing that you version in your system is 0.9.4, the checkout command must be:

    ```
    git checktout v0.9.4
    ```
        
2. Cloning the plug-ins repository:

      ```
      git clone https://github.com/pgmodeler/plugins.git
      ```

3. Running ```qmake``` with custom parameters in order to allow standalone build:

    ```
    $QT_ROOT\bin\qmake plugins.pro PGMODELER_PLUGINS=$PGMODELER_PLUGINS_ROOT
                                   PGMODELER_SRC=$PGMODELER_SOURCE_ROOT
                                   PGMODELER_LIBS=$PGMODELER_LIBRARIES_ROOT
    ```
    
    In the command above, the variable ```$QT_ROOT``` must be replaced by the path to the binaries of your Qt installation.
    
    The argument ```PGMODELER_PLUGINS``` must point to the path where all pgModeler plug-ins are stored in your installation, also know as, plug-ins root directory. If you don't know where they are install, just run pgModeler, open the settings dialog, and locate the [Plug-ins settings](https://www.pgmodeler.io/support/docs/46-plug-ins-settings?v=0.9.4). In the command above, replace ```$PGMODELER_PLUGINS_ROOT``` by the path presented in the field **Plug-ins root directory** on plug-ins settings.

    The argument ```PGMODELER_SRC``` must point to the pgModeler source code (either that you've cloned or extracted). Don't forget to replace the variable ```$PGMODELER_SOURCE_ROOT``` by that path.

    Finally, the argument ```PGMODELER_LIBS``` must point to the path where all shared libraries (.so, .dll or .dylib) related to pgModeler are installed. Replace the variable ```$PGMODELER_LIBRARIES_ROOT``` by that path.
    
4. Build and install the plug-ins. From the folder where the plug-ins repository was clone, run:

    ```
    make && make install
    ```
   
 If the build process succeeds, will you see that some files are installed in the plug-ins root directory in your pgModeler installation. Now, just run pgModeler to test if the plug-ins are working properly. In case of any problem with any plug-in, pgModeler will refuse to load it and present a detailed error dialog. You can use the presented information to ask for help.


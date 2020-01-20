A plugin for [pgModeler](https://www.pgmodeler.io), the ERD tool for PostgreSQL.

The query builder v0.9.2 can now be considered in beta state, please help fix bugs reporting at [github](https://github.com/pgmodeler/plugins/issues).

# Description

pgModeler has reached a certain level of maturity.

While it has grown into a good database management software, it focuses primarily on conception and modelling, with its mastery of the Data Definition Language SQL subset, which help database architects get to a rigorous implementation.

This plugins gives a shot at the Data Query Language SQL subset.
It is aimed primarily at :
 - database analysts, IT or business advisors whom do not have high SQL skills. It may allow them to create queries from visual objects, and help them get on-board with SQL basics along the way.
 - folks with advanced SQL skills whom frequently write long, tedious and repetitive queries for their applications.

# Features
### Create SQL queries graphically.

<img src="https://github.com/Maxzor/pgmodeler_plugins_media/blob/master/builder.gif" width="550" height="300" />

[Explanatory video](http://www.youtube.com/watch?v=6e66-fNhvAY)

This is the traditional feature set, you can insert tables, columns, relations in the builder, group, sort them...

The plugin will then output the SQL, and allow you to save the code, or run it directly from the management section.

### Automatic-join mode.

<img src="https://github.com/Maxzor/pgmodeler_plugins_media/blob/master/inference.gif" width="550" height="300" />

[Explanatory video](http://www.youtube.com/watch?v=_5QNFXA03Y4)

This is a primer in the FLOSS world!
You get candidate join paths, ranked by score, for the items you inserted in the "SELECT" clause of the query.
This can prove useful in discovering a new database, previously reverse-engineered from production (pgModeler can do that !).
To be of any interest, the database and/or the model shall have foreign-key relationships declared. Otherwise, you can beforehand look at tools such as [linkifier](https://github.com/janmotl/linkifier).

This graphical query builder relies on graph algorithms, mainly Dijkstra's path-finding and Dreyfus-Wagner for Steiner trees. See the video above for more details.
Some [implementation research history](https://stackoverflow.com/questions/56193619/what-is-needed-to-use-bgl-algorithms-on-existing-data-structures-edges-and-ver).

Beware that for large models, or numerous columns selected, the solver can turn your computer into a heat machine and hang for a while!

A few white papers about SQL-join solvers :
- http://resources.mpi-inf.mpg.de/yago-naga/naga/download/ICDEResearchLong09_264.pdf
- https://www.ics.uci.edu/~chenli/pub/sigmod2009-tastier.pdf
- https://researcher.watson.ibm.com/researcher/files/in-diptsaha/sap-nlq.pdf
- http://www.vldb.org/pvldb/vol6/p1222-guerra.pdf
- https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/icde07steiner.pdf

# Installation
## Core-only :
There is __no need to do anything special__ to build the core of the query builder. Once you have cloned the plugin subproject into the root of the source-tree of pgmodeler, you can __run qmake normally__.

## SQL-join solver :
By default, the query builder is built without its SQL-join solver. The solver has a heavy dependency : the [Practical Aproximations Algorithms Library](http://paal.mimuw.edu.pl/) (paal), and paal depends itself onto the famous [C++ boost library](https://www.boost.org/) (paal might even get merged into it at some point).

There are multiple ways of building the solver, between being _fully assisted_, or doing it _completely manually_.
Consider that even if you want to get maximum guiding as we will see below (`qmake -r CONFIG+=INTERACTIVE_QMAKE`), it is better to follow next manual point for boost. (If you ask the guided way for boost, it will clone the entire boost repo.)

##### Preparing the dependencies
###### Boost
You will need boost. The recommended path to get it is to use the one shipped in your system.

- GNU/Linux - Get it from your package manager, for example `sudo apt install libbost-dev`.
- Windows - Getting boost can be done from minGW repository. In Msys2's minGW64 console : `pacman -S mingw-w64-x86_64-boost`.
- Mac - `brew install boost`

You can otherwise install the pre-compiled libraries from https://boost.org, or, as is done in `setup.sh boost`, "build" boost from their git repo. This is a longer path. If you go the manual way, don't forget to add the path to boost in graphicalquerybuilder.pro.

###### Paal
Paal dependency will get cloned from its repository, and slightly tweaked (see what `setup.sh paal` does).
For paal you'd better follow the assisted way.

##### Preparing the build system
To tell qmake that you want to build the solver, you can either :
- set the variables in graphicalquerybuilder.conf before running qmake. This simply consists in replacing "y" by "n". E.g., from pgmodeler source root
    - you want to build the solver, `sed -i.bak s/GQB_JOIN_SOLVER=\"n\"/GQB_JOIN_SOLVER=\"y\"/ plugins/graphicalquerybuilder/graphicalquerybuilder.conf`.
    - you have boost installed, `sed -i.bak s/BOOST_INSTALLED=\"n\"/BOOST_INSTALLED=\"y\"/ plugins/graphicalquerybuilder/graphicalquerybuilder.conf`.
- get assisted throughout the setup adding the flag INTERACTIVE_QMAKE (the g++ flags down here can come in handy aswell). Still from pgmodeler source root :

`$QT_ROOT/bin/qmake -r CONFIG+=INTERACTIVE_QMAKE QMAKE_CXXFLAGS_WARN_ON="-Wall \-Wno-deprecated-declarations \-Wno-deprecated-copy" CONFIG+=release PREFIX=$INSTALLATION_ROOT BINDIR=$INSTALLATION_ROOT \
                         PRIVATEBINDIR=$INSTALLATION_ROOT PRIVATELIBDIR=$INSTALLATION_ROOT/lib pgmodeler.pro
`
Finally compile pgmodeler, referring to the [installation documentation](https://www.pgmodeler.io/support/installation).

##### Summing it up
If you have already configured your environment to build pgmodeler once, building the whole pgModeler project with the query builder core + solver, on a GNU/Linux station, should work with :
```
#Getting ready-to-go-boost
sudo apt install libbost-dev

#Getting the sources
git clone https://www.github.com/pgmodeler/pgmodeler
#Might need to git checkout the correct branch while all is not in master
cd pgmodeler
INSTALLATION_ROOT=$PWD #or wherever out-of-tree you want to
git clone https://www.github.com/pgmodeler/plugins
#Same as above

#Getting paal
HERE=$PWD
cd plugins/graphicalquerybuilder
./setup.sh paal
cd -

#Tweaking the conf file
sed -i.bak s/GQB_JOIN_SOLVER=\"n\"/GQB_JOIN_SOLVER=\"y\"/ plugins/graphicalquerybuilder/graphicalquerybuilder.conf
sed -i.bak s/BOOST_INSTALLED=\"n\"/BOOST_INSTALLED=\"y\"/ plugins/graphicalquerybuilder/graphicalquerybuilder.conf

#Running qmake
QT_ROOT= #type your Qt path here, where bin and include folders are
$QT_ROOT/bin/qmake -r CONFIG+=release PREFIX=$INSTALLATION_ROOT BINDIR=$INSTALLATION_ROOT \
                         PRIVATEBINDIR=$INSTALLATION_ROOT PRIVATELIBDIR=$INSTALLATION_ROOT/lib pgmodeler.pro

#Building
make && make install #Don't forget you can speed things up with parallelism :) make -j"nb of CPU cores"
```

# Contributing

_UI translations_ are appreciated, and you can help on [plenty other topics](https://github.com/pgmodeler/plugins/graphicalquerybuilder/CONTRIBUTING.md).

# Authors

 * **Maxime Chambonnet** - *Initial work*

See also the list of [contributors -soon you hopefully ? :)](https://github.com/pgmodeler/plugins/contributors) who participated in this project.

Thank you to Raphael Araújo e Silva, the author of pgModeler, for his great software that made this feature possible!

Enjoy!

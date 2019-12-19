A plugin for [Pgmodeler](https://www.pgmodeler.io), the ERD tool for Postgresql.

This is still alpha, expect bugs, and please help fix them reporting at :

https://gitlab.com/maxzor/pgmodeler_gqb

https://github.com/pgmodeler/plugins

# Features
Create SQL queries graphically.
Quick presentation : https://www.youtube.com/watch?v=6e66-fNhvAY

With an "automatic join" mode.
Join-path inference engine : https://www.youtube.com/watch?v=_5QNFXA03Y4

# Installation
Run the setup.sh shell script.

It will download the dependencies, [Paal](http://paal.mimuw.edu.pl/) and [Boost](https://www.boost.org/) recursively (expect tons of MBs of network usage...), and will configure the Qt build system to build the plugin.

`chmod +x setup.sh && ./setup.sh`

Move then the graphicalquerybuilder directory in the pgmodeler source tree, in its plugin directory.


Compile pgmodeler, enjoy!

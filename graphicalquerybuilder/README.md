A plugin for [Pgmodeler](https://www.pgmodeler.io), the ERD tool for Postgresql.

This is still alpha, expect bugs, and please help fix them reporting at [github](https://github.com/pgmodeler/plugins).

![Overview](res/overview.png)
![Overview_SQL](res/overview_sql.png)

# Features
### Create SQL queries graphically.

[![](http://img.youtube.com/vi/6e66-fNhvAY/0.jpg)](http://www.youtube.com/watch?v=6e66-fNhvAY "")

This is the traditional feature set, you can insert tables, columns, relations in the builder, group, sort them... and it outputs the DML SQL.

### Automatic-join mode.

[![](http://img.youtube.com/vi/_5QNFXA03Y4/0.jpg)](http://www.youtube.com/watch?v=_5QNFXA03Y4 "")

This is a primer in the FLOSS world. You get candidate join paths ranked by score for the items you inserted in the "SELECT" clause of the query.

# Installation
Run the setup.sh shell script.

It will download the plugin dependencies, [Paal](http://paal.mimuw.edu.pl/) and [Boost](https://www.boost.org/) recursively (expect tons of MBs of network usage...), and will configure Qt build system for the plugin.

`chmod +x setup.sh && ./setup.sh`

Move then the graphicalquerybuilder directory in the pgmodeler source tree, in its plugin directory.


Compile pgmodeler, enjoy!

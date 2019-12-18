# pgmodeler_gqb

A plugin for https://www.pgmodeler.io

Create SQL queries graphically.
With a path inference join solver.

Quick presentation : https://www.youtube.com/watch?v=6e66-fNhvAY

Join-path inference engine : https://www.youtube.com/watch?v=_5QNFXA03Y4


INSTALLATION

Download paal, and boost (I did download them separately)

For paal http://paal.mimuw.edu.pl/, in the graphicalquerybuilder directory :
git clone http://siekiera.mimuw.edu.pl:8082/paal
Adjust the paal.pro boost includes ;
then move paal.pro in the paal directory you just cloned.

Move then the graphicalquerybuilder directory in the pgmodeler source tree, in the plugin directory.

Compile pgmodeler (with release mode to build the plugins, for now).


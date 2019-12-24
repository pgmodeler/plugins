Some ideas, which might be transformed to reality, and that you are very welcome to tackle :

Sad limitations
 - No support for aliases.
 - No support in the join solver for parallel edges.
 - Support more types of relations in the join path. (currently 1-1, 1-n, n-n)
 - The SQL-solver performance scales quite bad, it is really terrible on the number of tables you want to select. Past 5 it often becomes untractable. Re-engineering, optimizations are on the way, as well as a progress bar and a cancel button.

Appearance
 - General UI beautification, and UX enhancements

SQL generation
 - Move, from the current 'flat' generation, to the pgmodeler micro-language.
 Needs to develop this last one with loops, notably.
 - Sql validation
 - Proper disambiguation between schemas, tables, columns.

Graph algorithms
 - Write a proper longest-path solver (used to explore exhaustively the search space)
 - Rewrite the k+1 steiner tree algo which scales really bad.
 - Get rid of external dependencies ?

General
 - Isolate query : move all non query items to a temp invisible layer.
 - Add support for temporary created gqb-join relations +++
 - Add support for multiple instances of a same item - via (table) aliases +++
 - Graphically select the instance of the table you want to connect the rel to ++
The features above could use as prerequisite the [multi position layers idea](https://github.com/pgmodeler/pgmodeler/issues/1318).

 - Similar to the core-code model-tree states, keep track of queries in each opened model.
Changing model would preserve the queries being built, and not delete them like in the current implementation.
 - Query tabs, might use the 1.0 pgmodeler rework with Qt dock widgets.

 - Subqueries, and navigation between levels of a nested query (in from, where...) ++
 - Reverse-engineering of a DQL statement ++
 - Complete DQL support : recursive, lateral, limit... ++
 - Support for full DML building : insert, update, delete, upsert... +
 - Expression engine / built-in/type-specific functions -
 - Pivot queries

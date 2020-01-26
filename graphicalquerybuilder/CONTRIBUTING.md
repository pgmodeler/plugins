Some ideas, which might be transformed to reality, and that you are very welcome to tackle :

# Main priorities
 - Support for aliases.
 - Support more types of relations in the join path (currently 1-1, 1-n, n-n).
 - Support in the join solver for parallel edges.

# Further due
### Appearance
 - General UI beautification, and UX enhancements

### SQL generation
 - Turn the SQL popup into a pane.
 - Move, from the current 'flat' generation, to the pgmodeler micro-language.
 Needs to develop this last one with loops, notably.
 - Branch a proper sql formatter, such as pgFormatter or SQLInForm. Need to discuss with @rkhaotix.
 - Sql validation
 - Proper disambiguation between schemas, tables, columns.

### Graph algorithms
 - Write a proper longest-path solver (used to explore exhaustively the search space)
 - Rewrite the k+1 steiner tree algo which scales quite poorly.
 I see two options : throw compute capacity at the problem, the current algo can be easily parallelized and multithreaded. I think this is not the right way, it would be cleaner to dive in the algorithm and write a better "k+1 optimal Dreyfus Wagner", with integrated sub-edge calculus and predecessor maps.
 - Get rid of external dependencies ? With such above rewrite, it would make sense.

### General
 - Isolate query : move all non query items to a temp invisible layer.
 - Add support for temporary-created relations +++
 - Add support for multiple instances of a same item - via (table) aliases +++
 - Graphically select the instance of the table you want to connect the rel to ++
The features above could use as prerequisite the [multi position layers idea](https://github.com/pgmodeler/pgmodeler/issues/1318).

 - Similar to pgModeler's model-tree states, keep track of queries in each opened model.
Changing model would preserve the queries being built, and not delete them like in the current implementation.
 - Query tabs, might use the 1.0 pgmodeler rework with KDAB's advanced dock widgets.
 - Subqueries, and navigation between levels of a nested query (in from, where...) ++
 - Reverse-engineering of a DQL statement ++
 - Complete DQL support : recursive, lateral, limit... ++
 - Support for full DML building : insert, update, delete, upsert... +
 - Expression engine / built-in/type-specific functions -
 - Pivot queries

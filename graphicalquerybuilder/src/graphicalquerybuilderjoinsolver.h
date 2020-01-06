/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2018 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

/**
\ingroup plugins/graphicalquerybuilder
\class QueryBuilderJoinSolver
\brief Join path solver for the graphical query builder.
	Implements the main algorithms. See project README.md
*/

#ifndef GRAPHICALQUERYBUILDERJOINSOLVER_H
#define GRAPHICALQUERYBUILDERJOINSOLVER_H

#include <QObject>
#include "basetable.h"
#include "baserelationship.h"
#include "paal/data_structures/metric/graph_metrics.hpp"
#include "paal/steiner_tree/dreyfus_wagner.hpp"
#include <QMetaType>
typedef QMultiMap<int,
QPair<
	  QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
	  QVector<QPair<BaseRelationship*, int>
>>> paths;

class GraphicalQueryBuilderPathWidget;

class GraphicalQueryBuilderJoinSolver: public QObject{

	//Aliases for boost and paal structures.
	using EdgeProp = boost::property<boost::edge_weight_t, int>;
	using Graph = boost::adjacency_list<
		boost::vecS, boost::vecS, boost::undirectedS,
		boost::property<boost::vertex_color_t, int>, EdgeProp>;
	using Edge = QPair<int, int>;
	using GraphMT = paal::data_structures::graph_metric<Graph, int>;
	using Terminals = std::vector<int>;
	using edge_parallel_category = boost::allow_parallel_edge_tag;
	using CostMap=paal::data_structures::graph_metric<Graph,
			int, paal::data_structures::graph_type::sparse_tag>;
	using Path = QVector<Edge>;

	private:
		Q_OBJECT

		GraphicalQueryBuilderPathWidget *gqb_p;

		//! \brief Indicates if the solver run was stopped by the user
		bool stop_solver_requested;

		//! \brief Multiply each super-edge possibilities between each other.
		void cartesianProductOnSuperEdges(
				QVector<QPair<
					QVector<Path>,QVector<QVector<int>>>>& v,
				QVector<int>& steiner_points,
				QHash<Edge, QPair<BaseRelationship*, int>>& edges_hash,
				QHash<int, BaseTable*>& tables_r,
				QMultiMap<int,
					QPair<
						QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
						QVector<QPair<BaseRelationship*, int>
				>>>& super_res
				);

	public:
		GraphicalQueryBuilderJoinSolver(GraphicalQueryBuilderPathWidget *widget);

		//! \k+1 shortest paths.
		//! This will compute all the possible paths between two points
		//! for a given cost. Used a lot in findPath().
		QPair<QVector<Path>, QVector<QVector<int>>> getDetailedPaths(Edge edge,
										QVector<int> terminals,
										int cost,
										CostMap &cost_map,
										QHash<BaseTable*, int> &tables,
										QHash<Edge, QPair<BaseRelationship*, int>> &edges_hash,
										int mode);


	public slots:
		//! \brief The main path inference engine, uses paal/boost
		void findPaths(void);

		void handleJoinSolverStopRequest(void);

	private slots:

	signals:

		void s_progressUpdated(short mode, int st_round, short powN, int st_comb, int st_found,
							   int sp_current, int sp_current_on, int sp_found,
							   int st_fround, int mult_entry, int mult_entry_on, int paths_found);

		void s_pathsFound(paths paths_found);
		void s_solverStopped(void);

	friend class GraphicalQueryBuilderPathWidget;
};

#endif // GRAPHICALQUERYBUILDERJOINSOLVER_H

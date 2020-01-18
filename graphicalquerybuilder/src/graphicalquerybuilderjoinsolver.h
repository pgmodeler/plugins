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

//! \brief This needs registration to get communicated as argument between threads.
typedef QMultiMap<int,
QPair<
	  QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
	  QVector<QPair<BaseRelationship*, int>
>>> paths;
typedef QVector<BaseTable *> bts;


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

		QThread *this_thread;
		bool real_time_rendering;
		int delay;

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
		GraphicalQueryBuilderJoinSolver(GraphicalQueryBuilderPathWidget *widget,
										QThread *thread, bool real_time_rendering, int delay);

		static constexpr unsigned
		PT_SR=0,	//Steiner points
		PT_SP1=1,	//Source and target
		PT_SP2=2,	//Predecessor map
		PT_FR1=3,	//Steiner points
		PT_FR2=4;	//Involved tables non steiner

		//! \brief Aliases for the progress reports
		static constexpr unsigned
			Progress_ShortPathMod0=0,	//Two tables to join
			Progress_SteinerRound=1,	//k+1-Steiner round
			Progress_SteinerComb=2,		//k+1-Steiner combination
			Progress_SuperEdgeRound=3,
			Progress_ShortPathMod1=4,	// sub-paths found
			Progress_FinalRound1=5,		//multiplication a
			Progress_FinalRound2=6,		//multiplication b
			Progress_FinalRound3=7,		//multiplication c
			Progress_FinalRound4=8;		//multiplication d

		//! \k+1 shortest paths.
		//! This will compute all the possible paths between two points
		//! for a given cost. Used a lot in findPath().
		QPair<QVector<Path>, QVector<QVector<int>>> getDetailedPaths(Edge edge,
										QVector<int> terminals,
										int cost,
										CostMap &cost_map,
										QHash<BaseTable*, int> &tables,
										QHash<Edge, QPair<BaseRelationship*, int>> &edges_hash,
										int mode,
										QHash<int, BaseTable*> &tables_r);


	public slots:
		//! \brief The main path inference engine, uses paal/boost
		void findPaths();

		//! \brief simply sets the attribute stop_solver_requested to true,
		//! it will be checked regularly during the solver run.
		void handleJoinSolverStopRequest(void);

	private slots:

	signals:

		//! \brief Progress reports, sent at various steps of the solver run,
		//! to the pathwidget status tab.
		void s_progressUpdated(short mode,
							   short st_round, short powN, long long st_comb, int st_found,
							   int sp_current, int sp_current_on, long long sp_found,
							   int st_fround, long long mult_entry, long long mult_entry_on, long long paths_found);

		void s_progressTables(int mode, bts btss);

		//! \brief Emitted when the solver is successful.
		void s_pathsFound(paths paths_found);

		//! \brief Emitted when the solver was canceled. It will allow thread->quit().
		void s_solverStopped(void);

	friend class GraphicalQueryBuilderPathWidget;
};

#endif // GRAPHICALQUERYBUILDERJOINSOLVER_H

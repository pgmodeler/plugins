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
\ingroup libpgmodeler_ui
\class QueryBuilderPathWidget
\brief Join path widget for the graphical query builder.
	allows for visualisation of paths.
	Paths can be inserted manually, or automatically via the inference engine.
*/

#ifndef GRAPHICALQUERYBUILDERPATHWIDGET_H
#define GRAPHICALQUERYBUILDERPATHWIDGET_H

#include "ui_graphicalquerybuilderpathwidget.h"
#include "pgmodelerns.h"
#include "modelwidget.h"
#include "basetable.h"
#include <QWidget>
#include "paal/data_structures/metric/graph_metrics.hpp"
#include "paal/steiner_tree/dreyfus_wagner.hpp"

class GraphicalQueryBuilderCoreWidget;

class GraphicalQueryBuilderPathWidget: public QWidget, public Ui::GraphicalQueryBuilderPathWidget {

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

		//! \brief A pointer to the sibling widget
		GraphicalQueryBuilderCoreWidget *gqb_c;

		//! \brief Reference model widget
		ModelWidget *model_wgt;

		QMenu reset_menu;

		static constexpr unsigned Manual=0,
		Automatic=1,
		Parameters=2;

		//! \brief Stores the selected path with two integers :
		//! 1 mode manual/auto 2 number of auto path
		QPair<int, int> path_mode_set;

		//! \brief Captures the ENTER press to execute search
		bool eventFilter(QObject *object, QEvent *event) override;

		vector<int> setupWeights(void);

	public:
		GraphicalQueryBuilderPathWidget(QWidget *parent = nullptr);

		//! \brief Sets the database model to work on
		void setModel(ModelWidget *model_wgt);

		ModelWidget * getModel(void){return model_wgt;}

		//! \brief Set the pointer to the sibling widget
		void setFriendWidget(GraphicalQueryBuilderCoreWidget *friend_wgt){gqb_c=friend_wgt;};

		//! \brief Insert in the manual tab the relations selected
		void insertManualRels(QMap<int, BaseObjectView *> qrels);

		//! \brief Insert in the auto tab the output of the inference engine
		void insertAutoRels(QMultiMap<int,
									QPair<
										QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
										QVector<QPair<BaseRelationship*, int>>
							>> &paths);

		//! \brief Transmits the path selected to the query builder core widget
		QMap<int, BaseRelationship *> getRelPath(void);

		void resetManualPath(void){manual_path_tw->setRowCount(0);};

		int manualPathSize(void){return manual_path_tw->rowCount();};

		//! \Intermediary between the output of the inference engine
		//!  and the path formatted for the visualisation table
		QVector<Path> getDetailedPaths(Edge edge,
										QVector<int> terminals,
										int cost,
										CostMap &cost_map,
										QHash<BaseTable*, int> &tables,
										QHash<Edge, QPair<BaseRelationship*, int>> &edges_hash);


	public slots:
		//! \brief The main path inference engine, uses paal/boost
		QMultiMap<int,
				QPair<
							QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
							QVector<QPair<BaseRelationship*, int>
				>>> findPath(void);

		void resetAutoPath(){auto_path_tw->setRowCount(0);};

	private slots:
		void resetPaths(void);
		void automaticPathSelected(int new_row, int new_column, int old_row=0, int old_column=0);

	signals:
	void s_visibilityChanged(bool);
	void s_automaticPathSelected(int);

	friend class GraphicalQueryBuilderCoreWidget;

};

#endif // GRAPHICALQUERYBUILDERPATHWIDGET_H

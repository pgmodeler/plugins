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
\class QueryBuilderCoreWidget
\brief Implements a graphical query builder.
*/

#ifndef GRAPHICALQUERYBUILDERCOREWIDGET_H
#define GRAPHICALQUERYBUILDERCOREWIDGET_H

#include "ui_graphicalquerybuildercorewidget.h"
#include "graphicalquerybuilderpathwidget.h"
#include "graphicalquerybuildersqlwidget.h"
#include "pgmodelerns.h"
#include "modelwidget.h"
#include "sourcecodewidget.h"
#include "basetable.h"
#include <QWidget>

class GraphicalQueryBuilderCoreWidget: public QWidget, public Ui::GraphicalQueryBuilderCoreWidget {
	private:
		Q_OBJECT

		//! \brief A pointer to the sibling widget
		GraphicalQueryBuilderPathWidget * gqb_j;

		QMenu reset_menu;

		//! \brief Reference model widget
		ModelWidget *model_wgt;

		//! \brief Containers for the custom DFS
		QVector <BaseTable *> required_vertices, visited_vertices, disconnected_vertices;

		//! \brief Those two containers are very similar, but are used at different times,
		//! and their respective flag represent very different information :
		//! - dfs_rels is used by Depth-First-Search (DFS) algorithm,
		//!   and its flag means 'already visited or not'.
		//! - dfs_result is used by the function getQueryPath,
		//!   and its 'flag' represents both the direction of the relation (see next constants)
		//!   from the query standpoint, aswell as if this relationship closes a cycle.
		QVector <QPair<bool, BaseRelationship *>> dfs_rels;
		QVector <QPair<short, BaseRelationship *>> dfs_result;

		static constexpr unsigned DFS_flowRel=0, /* in right order : rel src table is query src table */
		DFS_reverseRel=1, /* in reverse order : rel src table is query dst table */
		DFS_flowCycleRel=2,
		DFS_reverseCycleRel=3;

		//! \brief Stores wether the SQL code should make the joins with "JOIN" or "WHERE"
		bool join_in_where;

		QString msg;

		//! \brief Captures the ENTER press to execute search
		bool eventFilter(QObject *object, QEvent *event) override;

		void resizeEvent(QResizeEvent *event) override;

		//! \brief A custom algorithm that does topological ordering and connected components
		void customDepthFirstSearch(BaseTable * current_vertex);

		//! \brief Create a new query builder "data" column
		void initializeColumn(int col_nb, BaseObject *bObj);

		void updateRequiredVertices(void);
		QVector <BaseTable *> getRequiredVertices(void);

		void configureOrderBySpinBoxes(vector<int> ob_cols, int ob_col, int state);
		void swapOrderBySpins(int col, int new_value);
		void columnSelectChecked(int col, int state);

		//! \brief Puts all the query element together and spouts the SQL
		QString produceSQL(bool schema_qualified, bool compact_sql);

	public:
		//! \brief Constants for the table widget line numbers
		static constexpr unsigned tW_Selection=0,
		tW_Schema=1,
		tW_Table=2,
		tW_Column=3,
		tW_Alias=4,
		tW_Where=5,
		tW_Group=6,
		tW_Having=7,
		tW_Order=8;

		GraphicalQueryBuilderCoreWidget(QWidget *parent = nullptr);

		//! \brief Sets the database model to work on
		void setModel(ModelWidget *model_wgt);

		ModelWidget * getModel(void){return model_wgt;}

		//! \brief Wraps the depth-first search algorithm
		QVector < QPair< BaseTable *, QVector < QPair<Column *, Column *> > > >  getQueryPath(void);

		void setFriendWidget(GraphicalQueryBuilderPathWidget *friend_wgt){gqb_j=friend_wgt;}

		void updateRelLabel(void);


	public slots:
		void hide(void);
		void insertSelection(void);

		//! \brief Calls the SQL generation and a form to show the code
		void showSQL(void);
		void reloadSQL(GraphicalQueryBuilderSQLWidget * gqbs, bool schema_qualified, bool compact_sql);
		void resetQuery();

		//! \brief Syncronizes UI states between widgets
		void gqbPathWidgetToggled(bool change);

		//! \brief Custom algorithm that returns which tables
		//! are accessible from a starting point, through traversable relations
		tuple<QHash<BaseTable*, int> ,vector<QPair<int, int>>,
			QHash<QPair<int, int>, QPair<BaseRelationship*, int>>>
				getConnectedComponents(void);


	private slots:
		void selectAllItemsFromQuery(void);
		void highlightQueryColumn(int col);
		void rearrangeTabSections(int log, int oldV, int newV);
		void orderByCountChanged(int ob_col, int state);

	signals:
		void s_visibilityChanged(bool);
		void s_gqbSqlRequested(QString query_txt);
		void s_joinPathToggled(bool);
		void s_adjustViewportToItems(QList<BaseObjectView *>);

	friend class GraphicalQueryBuilderPathWidget;
};

#endif // GRAPHICALQUERYBUILDERCOREWIDGET_H

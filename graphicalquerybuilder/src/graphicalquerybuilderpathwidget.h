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
\class QueryBuilderPathWidget
\brief Join path widget for the graphical query builder.
	allows for visualisation of paths.
	Paths can be inserted manually, or automatically via the inference engine.
*/

#ifndef GRAPHICALQUERYBUILDERPATHWIDGET_H
#define GRAPHICALQUERYBUILDERPATHWIDGET_H

#include "ui_graphicalquerybuilderpathwidget.h"
#include "coreutilsns.h"
#include "widgets/modelwidget.h"
#include "basetable.h"
#include <QWidget>

#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
#include "graphicalquerybuilderjoinsolver.h"
#include <QMetaType>
typedef QMultiMap<int,
QPair<
	  QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
	  QVector<QPair<BaseRelationship*, int>
>>> paths;
Q_DECLARE_METATYPE(paths);
typedef QVector<BaseTable *> bts;
Q_DECLARE_METATYPE(bts);
#endif

class GraphicalQueryBuilderCoreWidget;

class GraphicalQueryBuilderPathWidget: public QWidget, public Ui::GraphicalQueryBuilderPathWidget {

private:
		Q_OBJECT

		//! \brief A pointer to the sibling widget
		GraphicalQueryBuilderCoreWidget *gqb_c;

		//! \brief Reference model widget
		ModelWidget *model_wgt;

		QMenu reset_menu;

#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
		//! \brief The instance of the solver
		GraphicalQueryBuilderJoinSolver *join_solver;

		//! \brief Thread used to control the SQL-join computation
		QThread *join_solver_thread;

		QVector<QGraphicsItem *> pixs, tr_pixs;
#endif

		static constexpr unsigned Manual=0,
		Automatic=1,
		Parameters=2,
		SolverStatus=3;

		//! \brief Stores the selected path with two integers :
		//! 1 mode manual/auto 2 number of auto path
		QPair<int, int> path_mode_set;

		//! \brief Captures the ENTER press to execute search
		bool eventFilter(QObject *object, QEvent *event) override;

#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
		void createThread(void);
		void destroyThread(bool force);
		void runSQLJoinSolver(void);
		QGraphicsItem * addPix(QPointF pos, QColor col);
#endif

public:
		GraphicalQueryBuilderPathWidget(QWidget *parent = nullptr);

		//! \brief Sets the database model to work on
		void setModel(ModelWidget *model_wgt);

		ModelWidget * getModel(void){return model_wgt;}

		//! \brief Set the pointer to the sibling widget
		void setFriendWidget(GraphicalQueryBuilderCoreWidget *friend_wgt){gqb_c=friend_wgt;};

		//! \brief Insert in the manual tab the relations selected
		void insertManualRels(QMap<int, BaseObjectView *> qrels);

		//! \brief Transmits the path selected to the query builder core widget
		QMap<int, BaseRelationship *> getRelPath(void);

		void resetManualPath(void){manual_path_tw->setRowCount(0);};

		int manualPathSize(void){return manual_path_tw->rowCount();};

#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
		void resetJoinSolverStatus(void);
#endif

public slots:
#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
		void resetAutoPath(){auto_path_tw->setRowCount(0);};

		//! \brief Insert in the auto tab the output of the inference engine
		void insertAutoRels(paths paths_found);

		void handlePathsFound(paths p);
#endif
private slots:
		void resetPaths(void);
#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
		void automaticPathSelected(int new_row, int new_column, int old_row=0, int old_column=0);
		void updateProgress(short mode,
							short st_round, short powN, long long st_comb, int st_found,
							int sp_current, int sp_current_on, long long sp_found,
							int st_fround, long long mult_entry, long long mult_entry_on, long long paths_found);
		void stopSolver(void);
		void progressTables(int mode, bts btss);
#endif

signals:
		void s_visibilityChanged(bool);
    void s_adjustViewportToItems(QList<BaseObjectView *>);
#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
		void s_automaticPathSelected(int);
		void s_stopJoinSolverRequested(void);
#endif

	friend class GraphicalQueryBuilder;
	friend class GraphicalQueryBuilderCoreWidget;
#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
	friend class GraphicalQueryBuilderJoinSolver;
#endif
};

#endif // GRAPHICALQUERYBUILDERPATHWIDGET_H

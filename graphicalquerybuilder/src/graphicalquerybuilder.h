/*
# Projeto: Modelador de Banco de Dados PostgreSQL (pgModeler)
#
# Copyright 2006-2019 - Raphael Araújo e Silva <raphael@pgmodeler.io>
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
\ingroup graphicalquerybuilder
\class GraphicalQueryBuilder
\brief Plugin to create SQL queries graphically
*/

#ifndef GRAPHICAL_QUERY_BUILDER_H
#define GRAPHICAL_QUERY_BUILDER_H

#include "pgmodelerplugin.h"
#include "graphicalquerybuildercorewidget.h"
#include "graphicalquerybuilderpathwidget.h"

class GraphicalQueryBuilder: public QObject, public PgModelerPlugin {
	private:
		Q_OBJECT

		Q_PLUGIN_METADATA(IID "maxzor.eu.pgmodeler.graphical_query_builder" FILE "graphicalquerybuilder.json")

		//! \brief Declares the interface which is used to implement the plugin
		Q_INTERFACES(PgModelerPlugin)

	ModelWidget *current_model;

	bool is_plugin_active;

	/*
	 *  The two widgets gqbc_parent and gqbj_parent are dock widgets,
	 * they hold the actual query builder widgets, respectively gqb_core_widget and
	 * gqb_path_widget.
	 */
	QWidget *gqbc_parent, *gqbj_parent;
	GraphicalQueryBuilderCoreWidget *gqb_core_wgt;
	GraphicalQueryBuilderPathWidget *gqb_path_wgt;

	public:
		GraphicalQueryBuilder(void);

		QString getPluginTitle(void);
		QString getPluginVersion(void);
		QString getPluginAuthor(void);
		QString getPluginDescription(void);
		QKeySequence getPluginShortcut(void);
		bool hasMenuAction(void);
		void executePlugin(ModelWidget *model_wgt);		
		void initPlugin(MainWindow *main_window);

    private slots:
        void handleModelChange(ModelWidget *new_model);

	public slots:
		void showPluginInfo(void);

		//! \brief "SQL mode"
		void showGqbSql(QString query_txt);

		//! \brief Zoom and scroll the GraphicsView to match the bounding rect of the items
		void adjustViewportToItems(QList<BaseObjectView *> items);
};

#endif

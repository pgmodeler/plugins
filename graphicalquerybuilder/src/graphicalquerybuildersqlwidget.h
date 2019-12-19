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
\class QueryBuilderSqlWidget
\brief Implements the operations to visualize the sql of the graphical query.
*/

#ifndef GRAPHICALQUERYBUILDERSQLWIDGET_H
#define GRAPHICALQUERYBUILDERSQLWIDGET_H

#include "ui_graphicalquerybuildersqlwidget.h"
#include "baseobjectwidget.h"
#include "hinttextwidget.h"
#include "numberedtexteditor.h"

class GraphicalQueryBuilderSQLWidget: public BaseObjectWidget, public Ui::GraphicalQueryBuilderSqlWidget {
	private:
		Q_OBJECT

		static constexpr int OriginalSql=0,
		DependenciesSql=1,
		ChildrenSql=2;

		NumberedTextEditor *sqlcode_txt;

		SyntaxHighlighter *hl_sqlcode;

	public:
		GraphicalQueryBuilderSQLWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, BaseObject *object=nullptr);

		//This button sends SQL code to the manage section of pgmodeler
		void enableManageBtn(void);

		void displayQuery(QString query_txt);

		/* Forcing the widget to indicate that the handled object is not protected
		even if it IS protected. This will avoid the ok button of the parent dialog
		to be disabled */
		virtual bool isHandledObjectProtected(void){ return(false); }

	public slots:
		void applyConfiguration(void);

	private slots:
		//void generateSourceCode(int=0);
		void saveSQLCode(void);

	signals:
		void s_reloadSQL(GraphicalQueryBuilderSQLWidget * zis, bool schema_qualified, bool compact_sql);
		void s_sendToManage(QString query);
};

#endif // GRAPHICALQUERYBUILDERSQLWIDGET_H


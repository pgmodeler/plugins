/*
# PostgreSQL Database Modeler (pgModeler)
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

#include "graphicalquerybuilder.h"
#include "exception.h"
#include "messagebox.h"
#include "mainwindow.h"

GraphicalQueryBuilder::GraphicalQueryBuilder(void)
{
	configurePluginInfo(getPluginTitle(),
						getPluginVersion(),
						getPluginAuthor(),
						getPluginDescription(),

						GlobalAttributes::PluginsDir +
						GlobalAttributes::DirSeparator +
						QString("graphicalquerybuilder") +
						GlobalAttributes::DirSeparator + QString("graphicalquerybuilder.png"));

	is_plugin_active=false;
}

QString GraphicalQueryBuilder::getPluginTitle(void)
{
	return(trUtf8("Graphical query builder"));
}


QString GraphicalQueryBuilder::getPluginVersion(void)
{
	return(QString("0.9.0"));
}

QString GraphicalQueryBuilder::getPluginAuthor(void)
{
	return(QString("Maxime Chambonnet maxzor@maxzor.eu"));
}

QString GraphicalQueryBuilder::getPluginDescription(void)
{
	return(trUtf8("Check documentation, source code and report bugs at : \
				\n https://www.github.com/pgmodeler/plugins/graphicalquerybuilder \
				\n https://www.gitlab.com/maxzor/pgmodeler_gqb"));
}

void GraphicalQueryBuilder::showPluginInfo(void)
{
	plugin_info_frm->show();
}

void GraphicalQueryBuilder::initPlugin(QMainWindow *main_window)
{
	PgModelerPlugin::initPlugin(main_window);

	MainWindow *mw = dynamic_cast<MainWindow *>(main_window);

	QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Minimum);

	//Setup dock widgets
	gqbc_parent = new QWidget(mw->h_splitter2);
	gqbc_parent->setObjectName(QString::fromUtf8("gqbc_parent"));
	sizePolicy1.setHeightForWidth(gqbc_parent->sizePolicy().hasHeightForWidth());
	gqbc_parent->setSizePolicy(sizePolicy1);
	mw->h_splitter2->addWidget(gqbc_parent);

	gqbj_parent = new QWidget(mw->v_splitter_oprs_objs);
	gqbj_parent->setObjectName(QString::fromUtf8("gqbj_parent"));
	sizePolicy2.setHeightForWidth(gqbj_parent->sizePolicy().hasHeightForWidth());
	gqbj_parent->setSizePolicy(sizePolicy2);
	gqbj_parent->setMinimumSize(QSize(270, 0));
	mw->v_splitter_oprs_objs->addWidget(gqbj_parent);

	gqb_core_wgt=new GraphicalQueryBuilderCoreWidget;
	gqb_path_wgt=new GraphicalQueryBuilderPathWidget;

	QVBoxLayout *vlayout=new QVBoxLayout;
	vlayout=new QVBoxLayout;
	vlayout->setContentsMargins(0,0,0,0);
	vlayout->addWidget(gqb_path_wgt);
	gqbj_parent->setLayout(vlayout);

	QHBoxLayout * hlayout=new QHBoxLayout;
	hlayout=new QHBoxLayout;
	hlayout->setContentsMargins(0,0,0,0);
	hlayout->addWidget(gqb_core_wgt);
	gqbc_parent->setLayout(hlayout);

	//Setup the graphicalquerybuilder_core_widget pushbutton
	QToolButton *tb = new QToolButton(mw->tool_btns_bar_wgt);

	sizePolicy1.setHeightForWidth(tb->sizePolicy().hasHeightForWidth());
	tb->setSizePolicy(sizePolicy1);
	tb->setFocusPolicy(Qt::TabFocus);
	QIcon icon39;
	icon39.addFile(QString::fromUtf8(":/icones/icones/visaoarvore.png"), QSize(), QIcon::Normal, QIcon::Off);
	tb->setIcon(icon39);
	tb->setIconSize(QSize(22, 22));
	tb->setCheckable(true);
	tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	mw->horiz_wgts_btns_layout->addWidget(tb);
	tb->setText("Query builder");

	//Setup the graphicalquerybuilder_path_widget pushbutton
	QToolButton *tb2 = new QToolButton(mw->tool_btns_bar_wgt);

	sizePolicy1.setHeightForWidth(tb->sizePolicy().hasHeightForWidth());
	tb2->setSizePolicy(sizePolicy1);
	tb2->setFocusPolicy(Qt::TabFocus);
	icon39.addFile(QString::fromUtf8(":/icones/icones/visaoarvore.png"), QSize(), QIcon::Normal, QIcon::Off);
	tb2->setIcon(icon39);
	tb2->setIconSize(QSize(22, 22));
	tb2->setCheckable(true);
	tb2->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	mw->vert_wgts_btns_layout->addWidget(tb2);
	tb2->setText("GQB Path");


	//The following signals deal with the visibility states.
	connect(tb, SIGNAL(toggled(bool)), gqbc_parent, SLOT(setVisible(bool)));
	connect(tb, SIGNAL(toggled(bool)), gqb_core_wgt, SLOT(setVisible(bool)));
	connect(tb, SIGNAL(toggled(bool)), mw, SLOT(showBottomWidgetsBar(void)));
	connect(gqb_core_wgt, SIGNAL(s_visibilityChanged(bool)), tb, SLOT(setChecked(bool)));
	connect(gqb_core_wgt, SIGNAL(s_visibilityChanged(bool)), mw, SLOT(showBottomWidgetsBar()));


	connect(tb2, SIGNAL(toggled(bool)), gqbj_parent, SLOT(setVisible(bool)));
	connect(tb2, SIGNAL(toggled(bool)), gqb_path_wgt, SLOT(setVisible(bool)));
	connect(tb2, SIGNAL(toggled(bool)), mw, SLOT(showRightWidgetsBar(void)));
	connect(gqb_path_wgt, SIGNAL(s_visibilityChanged(bool)), tb2, SLOT(setChecked(bool)));
	connect(gqb_path_wgt, SIGNAL(s_visibilityChanged(bool)), mw, SLOT(showRightWidgetsBar()));

	/*
	 * There _seems_ to not exist bidirectional binding, even with the Qt property system
	 * https://bugreports.qt.io/browse/QTBUG-19892
	 * These two signals synchronize the visibility states between widgets and button.
	 */
	connect(gqb_core_wgt, &GraphicalQueryBuilderCoreWidget::s_joinPathToggled, [&, tb2](bool checked){
		tb2->setChecked(checked);
	});
	connect(gqb_path_wgt, SIGNAL(s_visibilityChanged(bool)), gqb_core_wgt, SLOT(gqbPathWidgetToggled(bool)));

	//Set to each gqb widget a pointer to its sibling.
	gqb_core_wgt->setFriendWidget(gqb_path_wgt);
	gqb_path_wgt->setFriendWidget(gqb_core_wgt);

	//The "SQL mode"
	connect(gqb_core_wgt, SIGNAL(s_gqbSqlRequested(QString)), this, SLOT(showGqbSql(QString)));


	connect(mw, &MainWindow::s_currentModelChanged, [&](ModelWidget *new_model){
		this->current_model=new_model;
		gqb_core_wgt->setModel(new_model);
		gqb_path_wgt->setModel(new_model);
		disconnect(gqb_core_wgt, SIGNAL(s_gqbSqlRequested(QString)), nullptr,nullptr);
		if(new_model)
			connect(gqb_core_wgt, SIGNAL(s_gqbSqlRequested(QString)), this, SLOT(showGqbSql(QString)));
	});

	connect(gqb_core_wgt, SIGNAL(s_adjustViewportToItems(QList<BaseObjectView *>)),
				this, SLOT(adjustViewportToItems(QList<BaseObjectView *>)));

#ifndef QT_NO_TOOLTIP
		tb->setToolTip(QApplication::translate("MainWindow", "Toggle the graphical query builder", nullptr));
#endif // QT_NO_TOOLTIP
		tb->setText(QApplication::translate("MainWindow", "Que&ry builder", nullptr));
#ifndef QT_NO_SHORTCUT
		tb->setShortcut(QApplication::translate("MainWindow", "Alt+R", nullptr));
#endif // QT_NO_SHORTCUT

	//Setup of the two dock widgets finished
	gqbc_parent->setVisible(false);
	gqbj_parent->setVisible(false);
	gqb_core_wgt->setVisible(false);
	gqb_path_wgt->setVisible(false);

}

void GraphicalQueryBuilder::executePlugin(ModelWidget *model_wgt)
{
	if(is_plugin_active)
	{
		Messagebox msgbox;
		msgbox.show(trUtf8("Plugin already loaded!"),
								Messagebox::InfoIcon);
		return;
	}

	Messagebox msgbox;
	msgbox.show(trUtf8("Plugin successfully loaded!"),
							Messagebox::InfoIcon);
	is_plugin_active=true;
}

void GraphicalQueryBuilder::showGqbSql(QString query_txt)
{
	MainWindow *mw = dynamic_cast<MainWindow *>(main_window);

	auto *querybuilder_sql_wgt=new GraphicalQueryBuilderSQLWidget;
	querybuilder_sql_wgt->displayQuery(query_txt);

	connect(querybuilder_sql_wgt, &GraphicalQueryBuilderSQLWidget::s_reloadSQL,
		[&](GraphicalQueryBuilderSQLWidget * gqbs, bool schema_qualified, bool compact_sql){
			gqb_core_wgt->reloadSQL(gqbs, schema_qualified, compact_sql);
		});

	QPair <bool, SQLToolWidget *> sql_tw_pair=mw->isAnyManageDbOpened();
	if (sql_tw_pair.first)
	{
		querybuilder_sql_wgt->enableManageBtn();
		connect(querybuilder_sql_wgt, &GraphicalQueryBuilderSQLWidget::s_sendToManage ,this, [&, sql_tw_pair, mw](QString query_text){
			sql_tw_pair.second->insertQuery(query_text);
			mw->switchView(2);
		});
	}
	current_model->openEditingForm(querybuilder_sql_wgt, Messagebox::OkButton);
}

void GraphicalQueryBuilder::adjustViewportToItems(QList<BaseObjectView *> items)
{
	QRectF new_pov=items.front()->sceneBoundingRect();
	for(const auto &item:items)
		new_pov=new_pov.united(item->sceneBoundingRect());
	current_model->getViewport()->fitInView(new_pov,Qt::KeepAspectRatio);
}


QKeySequence GraphicalQueryBuilder::getPluginShortcut(void)
{
	return(QKeySequence(QString("Ctrl+J")));
}

bool GraphicalQueryBuilder::hasMenuAction(void)
{
	return(false);
}

/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2024 - Raphael Araújo e Silva <raphael@pgmodeler.io>
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

#include "sampleplugin.h"
#include "exception.h"
#include "messagebox.h"
#include "mainwindow.h"

SamplePlugin::SamplePlugin(void)
{
	configurePluginInfo(getPluginTitle(),
						getPluginVersion(),
						getPluginAuthor(),
						getPluginDescription());

	config_action = nullptr;
	toolbar_action = nullptr;
	model_action = nullptr;
	dbexplorer_tb = nullptr;
}

SamplePlugin::~SamplePlugin()
{
	delete dbexplorer_tb;
}

void SamplePlugin::initPlugin(MainWindow *main_wnd)
{
	PgModelerPlugin::initPlugin(main_wnd);

	config_action = new QAction(QIcon(getPluginIcon(getPluginName())), tr("Sample plug-in"), this);
	toolbar_action = new QAction(QIcon(getPluginIcon(getPluginName())), tr("Sample plug-in"), this);
	model_action = new QAction(QIcon(getPluginIcon(getPluginName())), tr("Sample plug-in"), this);

	dbexplorer_tb = new QToolButton;
	dbexplorer_tb->setIcon(QIcon(getPluginIcon(getPluginName())));
	dbexplorer_tb->setText(tr("Sample plug-in"));
	dbexplorer_tb->setToolTip(dbexplorer_tb->text());

	connect(config_action, &QAction::triggered, this, &SamplePlugin::executeConfigAction);
	connect(toolbar_action, &QAction::triggered, this, &SamplePlugin::executeToolbarAction);
	connect(model_action, &QAction::triggered, this, &SamplePlugin::executeModelAction);
	connect(dbexplorer_tb, &QToolButton::clicked, this, &SamplePlugin::executeDbExplorerAction);
}

QString SamplePlugin::getPluginTitle(void) const
{
	return(tr("Sample Plug-in"));
}

QString SamplePlugin::getPluginVersion(void) const
{
	return(QString("0.1.0"));
}

QString SamplePlugin::getPluginAuthor(void) const
{
	return(QString("Raphael Araújo e Silva"));
}

QString SamplePlugin::getPluginDescription(void) const
{
	return(tr("This sample plug-in has the only purpose to serve as a template for the development of extended features for pgModeler based on the plug-in interface."));
}

void SamplePlugin::showPluginInfo(void) const
{
	plugin_info_frm->show();
}

void SamplePlugin::executeToolbarAction()
{
	Messagebox msgbox;
	msgbox.show(tr("Toolbar action triggered!"),
				tr("This action triggers a specific operation when the user clicks the toolbar action."),
				Messagebox::InfoIcon);
}

void SamplePlugin::executeModelAction()
{
	Messagebox msgbox;
	msgbox.show(tr("Model action triggered!"),
				tr("This action triggers a specific operation when the user clicks the action in the current model's context menu."),
				Messagebox::InfoIcon);
}

void SamplePlugin::executeConfigAction()
{
	Messagebox msgbox;
	msgbox.show(tr("Configuration action triggered!"),
				tr("This action triggers a specific operation when the user clicks the action in the plug-ins settings menu."),
				Messagebox::InfoIcon);
}

void SamplePlugin::executeDbExplorerAction()
{
	Messagebox msgbox;
	msgbox.show(tr("Database explorer button triggered!"),
				tr("This button triggers a specific operation when the user clicks it in any instance of database explorer."),
				Messagebox::InfoIcon);
}

QAction *SamplePlugin::getAction(ActionId act_id) const
{
	if(act_id == ModelAction)
		return model_action;

	if(act_id == ConfigAction)
		return config_action;

	return toolbar_action;
}

QToolButton *SamplePlugin::getToolButton() const
{
	return dbexplorer_tb;
}

/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2023 - Raphael Araújo e Silva <raphael@pgmodeler.io>
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
}

void SamplePlugin::initPlugin(MainWindow *main_wnd)
{
	PgModelerPlugin::initPlugin(main_wnd);

	config_action = new QAction(QIcon(getPluginIcon(getPluginName())), tr("Sample plug-in"), this);
	toolbar_action = new QAction(QIcon(getPluginIcon(getPluginName())), tr("Sample plug-in"), this);
	model_action = new QAction(QIcon(getPluginIcon(getPluginName())), tr("Sample plug-in"), this);

	connect(config_action, &QAction::triggered, this, &SamplePlugin::executeConfigAction);
	connect(toolbar_action, &QAction::triggered, this, &SamplePlugin::executeToolbarAction);
	connect(model_action, &QAction::triggered, this, &SamplePlugin::executeModelAction);
}

QString SamplePlugin::getPluginTitle(void)
{
	return(tr("Sample Plug-in"));
}

QString SamplePlugin::getPluginVersion(void)
{
	return(QString("0.1.0"));
}

QString SamplePlugin::getPluginAuthor(void)
{
	return(QString("Raphael Araújo e Silva"));
}

QString SamplePlugin::getPluginDescription(void)
{
	return(tr("This sample plug-in has the only purpose to serve as a template for the development of extended features for pgModeler based on the plug-in interface."));
}

void SamplePlugin::showPluginInfo(void)
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

QAction *SamplePlugin::getToolbarAction()
{
	return toolbar_action;
}

QAction *SamplePlugin::getModelAction()
{
	return model_action;
}

QAction *SamplePlugin::getConfigAction()
{
	return config_action;
}

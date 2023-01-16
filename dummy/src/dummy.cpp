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

#include "dummy.h"
#include "exception.h"
#include "messagebox.h"
#include "mainwindow.h"

Dummy::Dummy(void)
{
	configurePluginInfo(getPluginTitle(),
						getPluginVersion(),
						getPluginAuthor(),
						getPluginDescription());

	config_action = new QAction();
	toolbar_action = new QAction();
	model_action = new QAction();

	connect(config_action, &QAction::triggered, this, &Dummy::executeConfigAction);
	connect(toolbar_action, &QAction::triggered, this, &Dummy::executeConfigAction);
	connect(model_action, &QAction::triggered, this, &Dummy::executeConfigAction);
}

void Dummy::initPlugin(MainWindow *main_wnd)
{
	PgModelerPlugin::initPlugin(main_wnd);
}

QString Dummy::getPluginTitle(void)
{
	return(tr("Dummy"));
}

QString Dummy::getPluginVersion(void)
{
	return(QString("0.1"));
}

QString Dummy::getPluginAuthor(void)
{
	return(QString("Raphael A. Silva"));
}

QString Dummy::getPluginDescription(void)
{
	return(tr("A dummy plugin only to test the pgModeler plugin structure."));
}

void Dummy::showPluginInfo(void)
{
	plugin_info_frm->show();
}

void Dummy::executeToolbarAction()
{
	Messagebox msgbox;
	msgbox.show(tr("Toolbar action triggered!"),
				tr("Plugin successfully loaded! Check the <a href='http://www.pgmodeler.com.br/wiki/doku.php?id=plugins'>plugins wiki page</a> to know how to create your own plugins."),
				Messagebox::InfoIcon);
}

void Dummy::executeModelAction()
{
	Messagebox msgbox;
	msgbox.show(tr("Model action triggered!"),
				tr("Plugin successfully loaded! Check the <a href='http://www.pgmodeler.com.br/wiki/doku.php?id=plugins'>plugins wiki page</a> to know how to create your own plugins."),
				Messagebox::InfoIcon);
}

void Dummy::executeConfigAction()
{
	Messagebox msgbox;
	msgbox.show(tr("Configuration action triggered!"),
				tr("Plugin successfully loaded! Check the <a href='http://www.pgmodeler.com.br/wiki/doku.php?id=plugins'>plugins wiki page</a> to know how to create your own plugins."),
				Messagebox::InfoIcon);
}

QAction *Dummy::getToolbarAction()
{
	return toolbar_action;
}

QAction *Dummy::getModelAction()
{
	return model_action;
}

QAction *Dummy::getConfigAction()
{
	return config_action;
}

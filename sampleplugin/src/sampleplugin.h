/*
# Projeto: Modelador de Banco de Dados PostgreSQL (pgModeler)
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

/**
\ingroup sampleplugin
\class SamplePlugin
\brief Example plugin for pgModeler (does not execute any complex operation)
*/

#ifndef SAMPLE_PLUGIN_H
#define SAMPLE_PLUGIN_H

#include "pgmodelerguiplugin.h"

class SamplePlugin: public QObject, public PgModelerGuiPlugin {
	private:
		Q_OBJECT

		Q_PLUGIN_METADATA(IID "io.pgmodeler.PgModelerGuiPlugin" FILE "sampleplugin.json")

		//! \brief Declares the interface which is used to implement the plugin
		Q_INTERFACES(PgModelerGuiPlugin)

		QAction *config_action, *toolbar_action, *model_action;

		QToolButton *dbexplorer_tb;

		void initPlugin(MainWindow *main_wnd) override;

	public:
		SamplePlugin();
		~SamplePlugin();

		QString getPluginTitle() const override;
		QString getPluginVersion() const override;
		QString getPluginAuthor() const override;
		QString getPluginDescription() const override;

		QAction *getAction(ActionId act_id) const override;
		QToolButton *getToolButton() const override;

	public slots:
		void showPluginInfo(void) const override;

	private slots:
		void executeToolbarAction();
		void executeModelAction();
		void executeConfigAction();
		void executeDbExplorerAction();
};

#endif

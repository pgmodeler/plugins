/*
# Projeto: Modelador de Banco de Dados PostgreSQL (pgModeler)
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

/**
\ingroup sampleplugin
\class SamplePlugin
\brief Example plugin for pgModeler (does not execute any complex operation)
*/

#ifndef SAMPLE_PLUGIN_H
#define SAMPLE_PLUGIN_H

#include "pgmodelerplugin.h"

class SamplePlugin: public QObject, public PgModelerPlugin {
	private:
		Q_OBJECT

		Q_PLUGIN_METADATA(IID "br.com.pgmodeler.PgModelerPlugin" FILE "sampleplugin.json")

		//! \brief Declares the interface which is used to implement the plugin
		Q_INTERFACES(PgModelerPlugin)

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

		QAction *getToolbarAction() const override;
		QAction *getModelAction() const override;
		QAction *getConfigAction() const override;
		QToolButton *getDbExplorerButton() const override;

	public slots:
		void showPluginInfo(void) const override;

	private slots:
		void executeToolbarAction();
		void executeModelAction();
		void executeConfigAction();
		void executeDbExplorerAction();
};

#endif

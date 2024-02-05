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
\brief Example plugin for pgModeler CLI (does not execute any complex operation)
*/

#ifndef SAMPLE_CLI_PLUGIN_H
#define SAMPLE_CLI_PLUGIN_H

#include "pgmodelercliplugin.h"

class SampleCliPlugin: public QObject, public PgModelerCliPlugin {
	private:
		Q_OBJECT

		Q_PLUGIN_METADATA(IID "io.pgmodeler.PgModelerCliPlugin" FILE "samplecliplugin.json")

		//! \brief Declares the interface which is used to implement the plugin
		Q_INTERFACES(PgModelerCliPlugin)

	public:
		static const QString SampleCliOpt;

		SampleCliPlugin();
		~SampleCliPlugin();

		QString getPluginTitle() const override;
		QString getPluginVersion() const override;
		QString getPluginAuthor() const override;
		QString getPluginDescription() const override;

		attribs_map getShortOptions() const override;
		std::map<QString, bool> getLongOptions() const override;
		QStringList getOpModeOptions() const override;
		attribs_map getOptsDescription() const override;
		OperationId getOperationId() const override;

		void runPreOperation() override;
		void runOperation() override;
		void runPostOperation() override;
};

#endif

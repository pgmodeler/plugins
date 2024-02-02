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

#include "samplecliplugin.h"
#include "pgmodelercliapp.h"

SampleCliPlugin::SampleCliPlugin() : PgModelerCliPlugin()
{

}

SampleCliPlugin::~SampleCliPlugin()
{

}

QString SampleCliPlugin::getPluginTitle() const
{
	return(tr("Sample Plug-in"));
}

QString SampleCliPlugin::getPluginVersion() const
{
	return("0.1.0");
}

QString SampleCliPlugin::getPluginAuthor() const
{
	return("Raphael Araújo e Silva");
}

QString SampleCliPlugin::getPluginDescription() const
{
	return(tr("This sample plug-in has the only purpose to serve as a template for the development of extended features for pgModeler based on the plug-in interface."));
}

attribs_map SampleCliPlugin::getPluginShortOpts() const
{
	return {{ "--sample-cli", "-sc" }};
}

std::map<QString, bool> SampleCliPlugin::getPluginLongOpts() const
{
	return {{ "--sample-cli", true }};
}

attribs_map SampleCliPlugin::getOptsDescription() const
{
	return {{ "--sample-cli", tr("A description for an option used by the samplecliplugin.") }};
}

PgModelerCliPlugin::TaskId SampleCliPlugin::getTaskId() const
{
	return CustomTask;
}

PgModelerCliPlugin::EventId SampleCliPlugin::getEventId() const
{
	return None;
}

void SampleCliPlugin::executePlugin()
{

}

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
#include "exception.h"

SampleCliPlugin::SampleCliPlugin() : PgModelerCliPlugin()
{

}

SampleCliPlugin::~SampleCliPlugin()
{

}

QString SampleCliPlugin::getPluginTitle(void) const
{
	return(tr("Sample Plug-in"));
}

QString SampleCliPlugin::getPluginVersion(void) const
{
	return(QString("0.1.0"));
}

QString SampleCliPlugin::getPluginAuthor(void) const
{
	return(QString("Raphael Araújo e Silva"));
}

QString SampleCliPlugin::getPluginDescription(void) const
{
	return(tr("This sample plug-in has the only purpose to serve as a template for the development of extended features for pgModeler based on the plug-in interface."));
}

attribs_map SampleCliPlugin::getPluginShortOpts() const
{
	return {};
}

std::map<QString, bool> SampleCliPlugin::getPluginLongOpts() const
{
	return {};
}

PgModelerCliPlugin::OperationId SampleCliPlugin::getOperationId()
{
	return CustomMode;
}

PgModelerCliPlugin::EventId SampleCliPlugin::getEventId()
{
	return None;
}

void SampleCliPlugin::executePlugin()
{

}

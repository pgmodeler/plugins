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

#include "graphicalquerybuildersqlwidget.h"
#include "taskprogresswidget.h"
#include "pgmodeleruins.h"
#include "sqltoolwidget.h"
#include "baseform.h"

GraphicalQueryBuilderSQLWidget::GraphicalQueryBuilderSQLWidget(QWidget *parent): BaseObjectWidget(parent)
{
	try
	{
		Ui_GraphicalQueryBuilderSqlWidget::setupUi(this);
		configureFormLayout(Ui_GraphicalQueryBuilderSqlWidget::gridLayout, ObjectType::BaseObject);
		obj_icon_lbl->setPixmap(PgModelerUiNs::getIconPath("visaoarvore"));
		this->name_edt->setText("Query builder output");

		comment_lbl->setVisible(false);
		comment_edt->setVisible(false);

		hl_sqlcode=nullptr;
		sqlcode_txt=PgModelerUiNs::createNumberedTextEditor(sqlcode_wgt);
		sqlcode_txt->setReadOnly(false);

		connect(save_sql_tb, SIGNAL(clicked()), this, SLOT(saveSQLCode()));
		connect(manage_tb, &QToolButton::clicked, [this](){
			emit s_sendToManage(sqlcode_txt->toPlainText());
			dynamic_cast<BaseForm *>(this->parentWidget()->parentWidget())->done(2);
			});
		connect(schem_qualif_tb, &QToolButton::clicked, [&](bool clicked){
			emit s_reloadSQL(this, join_in_where_chb->isChecked(), clicked, (bool)code_options_cmb->currentIndex());
		});
		connect(code_options_cmb, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index){
			emit s_reloadSQL(this, join_in_where_chb->isChecked(), schem_qualif_tb->isChecked(),(bool)index);
		});
		connect(join_in_where_chb, &QCheckBox::toggled, [&](bool change){
			emit s_reloadSQL(this, change, schem_qualif_tb->isChecked(),(bool)code_options_cmb->currentIndex());
		});


		hl_sqlcode=new SyntaxHighlighter(sqlcode_txt);

		setMinimumSize(640, 540);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void GraphicalQueryBuilderSQLWidget::saveSQLCode(void)
{
	QFileDialog file_dlg;

	file_dlg.setWindowTitle(tr("Save SQL code as..."));

	file_dlg.setFileMode(QFileDialog::AnyFile);
	file_dlg.setAcceptMode(QFileDialog::AcceptSave);
	file_dlg.setModal(true);
	file_dlg.setNameFilter(tr("SQL code (*.sql);;All files (*.*)"));
	file_dlg.selectFile(name_edt->text().simplified().replace(" ","_") + QString(".sql"));


	if(file_dlg.exec()==QFileDialog::Accepted)
	{
		QFile out;
		QByteArray buf;

		if(!file_dlg.selectedFiles().isEmpty())
		{
			out.setFileName(file_dlg.selectedFiles().at(0));

			if(!out.open(QFile::WriteOnly))
				throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(file_dlg.selectedFiles().at(0)),
												ErrorCode::FileDirectoryNotWritten,__PRETTY_FUNCTION__,__FILE__,__LINE__);

			buf.append(sqlcode_txt->toPlainText());
			out.write(buf.data(), buf.size());
			out.close();
		}
	}
}

void GraphicalQueryBuilderSQLWidget::applyConfiguration(void)
{
	emit s_closeRequested();
}

void GraphicalQueryBuilderSQLWidget::displayQuery(QString query_txt)
{
	//load config
	try
	{
		this->protected_obj_frm->setVisible(false);
		this->obj_id_lbl->setVisible(true);
		this->code_options_cmb->setEnabled(true);

		if(!hl_sqlcode->isConfigurationLoaded())
			hl_sqlcode->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}

	//write sql and see
	try
	{
	sqlcode_txt->clear();

	sqlcode_txt->setPlainText(query_txt);

	save_sql_tb->setEnabled(!sqlcode_txt->toPlainText().isEmpty());

	if(sqlcode_txt->toPlainText().isEmpty())
		sqlcode_txt->setPlainText(tr("-- Something went wrong for _some_ reason --"));

	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
		
	}
}

void GraphicalQueryBuilderSQLWidget::enableManageBtn(void)
{
	manage_tb->setEnabled(true);
}

#ifndef XML2OBJECT_WIDGET_H
#define XML2OBJECT_WIDGET_H

#include "ui_xml2objectwidget.h"
#include "utils/syntaxhighlighter.h"
#include "databasemodel.h"
#include "operationlist.h"

class Xml2ObjectWidget: public QDialog, public Ui::Xml2ObjectWidget {
	private:
		Q_OBJECT
		SyntaxHighlighter *code_hl;
		DatabaseModel *model;
		OperationList *op_list;

	public:
		Xml2ObjectWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Widget);

	public slots:
		void show(DatabaseModel *model, OperationList *op_list);

	private slots:
		void loadXML(void);
		void clearSource(void);
		void generateObject(void);
};

#endif

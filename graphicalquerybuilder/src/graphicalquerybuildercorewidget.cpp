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

#include "graphicalquerybuildercorewidget.h"
#include "pgmodeleruins.h"

GraphicalQueryBuilderCoreWidget::GraphicalQueryBuilderCoreWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	tab_wgt->setAccessibleName("gqbc_tw"); //solves 'del' shortcut override in mainwindow

	connect(insert_btn, SIGNAL(clicked(bool)), this, SLOT(insertSelection(void)));
	connect(show_sql_btn, SIGNAL(clicked(bool)), this, SLOT(showSQL()));

	reset_menu.addAction(tr("All"), this, SLOT(resetQuery()));
	reset_menu.addAction(tr("Data"), this, SLOT(resetQuery()));
	reset_menu.addAction(tr("Join path"), this, SLOT(resetQuery()));
	reset_btn->setMenu(&reset_menu);

	connect(isolate_btn, SIGNAL(clicked(void)), this, SLOT(selectAllItemsFromQuery()));
	connect(path_btn, &QToolButton::clicked, [&](){emit s_joinPathToggled(path_btn->isChecked());});
	connect(hide_tb, SIGNAL(clicked(void)), this, SLOT(hide(void)));
	connect(this->tab_wgt->horizontalHeader(), SIGNAL(sectionMoved(int , int , int )),
			this, SLOT(rearrangeTabSections(int, int , int )));

	this->setModel(nullptr);
	this->installEventFilter(this);

	this->tab_wgt->horizontalHeader()->setSectionsMovable(true);
	this->tab_wgt->verticalHeader()->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectColumns);
	this->tab_wgt->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	connect(tab_wgt, &QTableWidget::itemDoubleClicked, [&](QTableWidgetItem *item){
		if(!(item->flags() & Qt::ItemIsEditable))
			highlightQueryColumn(item->column());});

	rel_cnt_lbl->setVisible(false);
}

bool GraphicalQueryBuilderCoreWidget::eventFilter(QObject *object, QEvent *event)
{
	auto *k_event=dynamic_cast<QKeyEvent *>(event);

	//Show sql when user presses enter/return on the pattern field
	if(event->type() == QEvent::KeyPress &&
			(k_event->key()==Qt::Key_Return || k_event->key()==Qt::Key_Enter))
	{
		showSQL();
		return(true);
	}

	//Delete query items with del key
	if(k_event!=nullptr && k_event->key()==Qt::Key_Delete)
	{
		QMap<int, int> ord_sel_cols;
		for(auto sel_item:tab_wgt->selectedItems())
			ord_sel_cols.insert(tab_wgt->column(sel_item), tab_wgt->column(sel_item));
		QMapIterator<int, int> ord_sel_cols_itr(ord_sel_cols);
		ord_sel_cols_itr.toBack();
		while (ord_sel_cols_itr.hasPrevious())
		{
			ord_sel_cols_itr.previous();

			bool order_spin_vis=false;
			int order_spin_value;
			if( (order_spin_vis=qobject_cast<QSpinBox *>(
						tab_wgt->cellWidget(tW_Order,ord_sel_cols_itr.value())->children()[2])->isVisible()) )
			{
				order_spin_value = qobject_cast<QSpinBox *>(
						tab_wgt->cellWidget(tW_Order,ord_sel_cols_itr.value())->children()[2])->value();
			}

			tab_wgt->removeColumn(ord_sel_cols_itr.value());

			if(order_spin_vis)
				orderByCountChanged(order_spin_value,-1);
		}
		return true;
	}

	return(QWidget::eventFilter(object, event));
}

void GraphicalQueryBuilderCoreWidget::hide(void)
{
	QWidget::hide();
	emit s_visibilityChanged(false);
}

void GraphicalQueryBuilderCoreWidget::resizeEvent(QResizeEvent *event)
{
	Qt::ToolButtonStyle style=Qt::ToolButtonTextBesideIcon;

	if(event->size().width() < this->baseSize().width())
	{
		style=Qt::ToolButtonIconOnly;
		//rel_cnt_lbl->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	}
	//else
		//rel_cnt_lbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	insert_btn->setToolButtonStyle(style);
	show_sql_btn->setToolButtonStyle(style);
	reset_btn->setToolButtonStyle(style);
	isolate_btn->setToolButtonStyle(style);
	path_btn->setToolButtonStyle(style);
}

void GraphicalQueryBuilderCoreWidget::setModel(ModelWidget *model_wgt)
{
	bool enable=model_wgt!=nullptr;

	if(enable)
		this->resetQuery();

	this->model_wgt=model_wgt;

	insert_btn->setEnabled(enable);
	show_sql_btn->setEnabled(enable);
	tab_wgt->setEnabled(enable);
}

void GraphicalQueryBuilderCoreWidget::resetQuery()
{
	if(!reset_menu.actions().contains(qobject_cast<QAction *>(sender())) ||
			qobject_cast<QAction *>(sender()) == reset_menu.actions().at(0))
	{
		tab_wgt->setColumnCount(0);
		gqb_j->resetManualPath();
#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
		gqb_j->resetAutoPath();
#endif
		gqb_j->path_sw->setCurrentIndex(0);
		gqb_j->path_mode_set=qMakePair<int,int>(GraphicalQueryBuilderPathWidget::Manual,0);
		rel_cnt_lbl->setVisible(false);
	}
	else if(qobject_cast<QAction *>(sender()) == reset_menu.actions().at(1))
	{
		tab_wgt->setColumnCount(0);
	}
	else if(qobject_cast<QAction *>(sender()) == reset_menu.actions().at(2))
	{
		gqb_j->resetManualPath();
#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
		gqb_j->resetAutoPath();
#endif
		gqb_j->path_sw->setCurrentIndex(0);
		gqb_j->path_mode_set=qMakePair<int,int>(GraphicalQueryBuilderPathWidget::Manual,0);
		rel_cnt_lbl->setVisible(false);
	}
	if(tab_wgt->columnCount()==0 && gqb_j->manualPathSize()==0)
		reset_btn->setEnabled(false);
}

void GraphicalQueryBuilderCoreWidget::insertSelection(void)
{
	QMap<int, BaseObjectView *> ord_query_data, ord_query_rels;

	if(!model_wgt)
		return;

	//Get the selected scene items ordered in two containers : one for data, one for relations.
	ord_query_data.clear();

	for(const auto& item: model_wgt->getObjectsScene()->selectedItems())
	{
		auto obj=dynamic_cast<BaseObjectView *>(item);
		switch (obj->getUnderlyingObject()->getObjectType())
		{
			case ObjectType::Table:
			//case ObjectType::View:
			case ObjectType::Column:
				ord_query_data.insert(static_cast<int>(obj->getSelectionOrder()), obj);
				break;

			case ObjectType::Relationship:
			case ObjectType::BaseRelationship:
				ord_query_rels.insert(static_cast<int>(obj->getSelectionOrder()), obj);
				break;

			default:
				break;
		}
	}

	//Iterate over the data container to populate the query table.
	QMap<int, BaseObjectView *>::iterator map_itr = ord_query_data.begin();
	tab_wgt->setUpdatesEnabled(false);
	tab_wgt->setSortingEnabled(false);

	for(map_itr=ord_query_data.begin() ; map_itr!=ord_query_data.end() ; map_itr++)
	{
		int col_nb=tab_wgt->columnCount();
		initializeColumn(col_nb, map_itr.value()->getUnderlyingObject());
	}

	tab_wgt->setUpdatesEnabled(true);
	tab_wgt->setSortingEnabled(false);
	this->tab_wgt->resizeColumnsToContents();

	//Update the relation part: send the data to the secondary widget, change label
	gqb_j->insertManualRels(ord_query_rels);
	updateRelLabel();

	//Enable the reset button if there is something
	this->reset_btn->setEnabled(!ord_query_data.empty() || !ord_query_rels.empty());
}

void GraphicalQueryBuilderCoreWidget::showSQL(void)
{
	emit s_gqbSqlRequested(this->produceSQL(true, false, true, false));
}

void GraphicalQueryBuilderCoreWidget::reloadSQL(GraphicalQueryBuilderSQLWidget * gqbs, bool join_in_where, bool schema_qualified, bool compact_sql)
{
	gqbs->displayQuery(this->produceSQL(false, join_in_where, schema_qualified, compact_sql));
}

QString GraphicalQueryBuilderCoreWidget::produceSQL(bool initial_warning, bool join_in_where_asked,
													bool schema_qualified, bool compact_sql)
{
	QString select_cl="SELECT ", from_cl="FROM ", where_cl="WHERE ", group_cl="GROUP BY ", having_cl="HAVING ",
			order_cl="ORDER BY ", result;
	msg=nullptr;

	if(tab_wgt->columnCount()==0)
	{
		result=("SELECT v_schema.table.Hello FROM world!");
		return result;
	}

	//'FROM' clause
	if(tab_wgt->columnCount()==1)
	{
		from_cl+= (schema_qualified ? tab_wgt->item(tW_Schema,0)->text() + "." : "") +
					tab_wgt->item(tW_Table,0)->text() + "\n";
	}

	else
	{
		join_in_where=false;
		QVector < QPair< BaseTable *, QVector < QPair<Column *, Column *> > > > path;
		QVector < QPair<Column *, Column *> >::iterator col_itr;

		path=getQueryPath();

		if(join_in_where && initial_warning)
			msg+="At least one <strong>cycle</strong> has been found in the join path : "
				 "'<strong>join</strong> will happen <strong>in where</strong>'.<br/><br/>";

		if(!join_in_where && join_in_where_asked)
			join_in_where=true;

		QVector < QPair< BaseTable *, QVector < QPair<Column *, Column *> > > >::iterator path_itr;
		for (path_itr=path.begin(); path_itr!=path.end(); path_itr++)
		{
			if(path.empty())
				break;

			if(path_itr==path.begin())
				from_cl+= (schema_qualified ? path.front().first->getSchema()->getName() + "." : "" )
							+ path.front().first->getName();
			else
			{

				if (join_in_where)
				{
					if(path_itr->first!=nullptr)
						from_cl+= (compact_sql ? ", " : ",\n\t\t") +
								(schema_qualified ? path_itr->first->getSchema()->getName() + "." : "" ) +
								path_itr->first->getName();

					for(col_itr=path_itr->second.begin(); col_itr!=path_itr->second.end(); col_itr++)
					{
						//TODO manage disambiguation of schema/table/column names globally
						if(col_itr->first->getName()!=col_itr->second->getName())
							where_cl+=(where_cl=="WHERE " ? "" : (compact_sql ? " AND " : "\n\t\tAND ")) +
									col_itr->first->getName() + "=" + col_itr->second->getName();
						else if(col_itr->first->getParentTable()->getName()!=col_itr->second->getParentTable()->getName())
							where_cl+=(where_cl=="WHERE " ? "" : (compact_sql ? " AND " : "\n\t\tAND ")) +
							  col_itr->first->getParentTable()->getName() + "." + col_itr->first->getName() + "=" +
							  col_itr->second->getParentTable()->getName() + "." + col_itr->second->getName();
						// /!\ else TODO
					}
				}
				else
				{

					from_cl+="\nJOIN " + (schema_qualified ? path_itr->first->getSchema()->getName() + "." : "") +
								path_itr->first->getName() + (compact_sql ? " " : "\n");

					for(col_itr=path_itr->second.begin(); col_itr!=path_itr->second.end(); col_itr++)
					{
						from_cl+=(col_itr==path_itr->second.begin() ? (compact_sql ? "ON " : "\t\tON ") :
																	  (compact_sql ? " AND " : "\n\t\tAND "));

						if(col_itr->first->getName()!=col_itr->second->getName())
							from_cl+=col_itr->first->getName() + "=" + col_itr->second->getName();
						else if(col_itr->first->getParentTable()->getName()!=col_itr->second->getParentTable()->getName())
							from_cl+=
							  col_itr->first->getParentTable()->getName() + "." + col_itr->first->getName() + "=" +
							  col_itr->second->getParentTable()->getName() + "." + col_itr->second->getName();
						// /!\ else todo
					}
				}
			}
		}

		if(!path.empty())
			from_cl+=(!disconnected_vertices.empty()?",\n":"\n");
		if(!disconnected_vertices.empty())
		{
			if(required_vertices.size()!=1 && initial_warning)
				msg+="A valid join <strong>path</strong> has <strong>not</strong> been <strong>found</strong> for some tables :<br/>"
					 "these tables will be joined with a <strong>sheer cartesian product</strong>!<br/><br/>";

			for(int k=0;k<disconnected_vertices.size();k++)
			{
				if(k==0 && !path.empty())
					from_cl+= compact_sql ? "" : "\t\t";
				from_cl+= (schema_qualified ? disconnected_vertices[k]->getSchema()->getName() + "." : "") +
							disconnected_vertices[k]->getName() + (k==disconnected_vertices.size()-1 ? "\n" :
																	(compact_sql?", ":",\n\t\t"));
			}
		}
	}

	map<int,int> group_by_cols,order_by_cols;
	for (int i=0; i<tab_wgt->columnCount();i++)
	{
		//'SELECT' clause
		if(qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Selection,i)->children().last())->checkState()==Qt::Checked)
				select_cl+= tab_wgt->item(tW_Table,i)->text() + "." +
						   tab_wgt->item(tW_Column,i)->text() +
						   (i==tab_wgt->columnCount()-1?"\n": (compact_sql? ", " : ",\n\t\t"));

		//'WHERE' clause
		if (tab_wgt->item(tW_Where,i) && tab_wgt->item(tW_Where,i)->text()!="")
				where_cl+= (where_cl=="WHERE "? "" : (compact_sql?" AND ":"\n\t\tAND ")) + tab_wgt->item(tW_Column,i)->text() +
							   tab_wgt->item(tW_Where,i)->text();

		//'GROUP BY' clause
		if(qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Group,i)->children()[1])->checkState()==Qt::Checked)
			group_cl+= (group_cl=="GROUP BY "? "" : (compact_sql?", ":",\n\t\t")) + tab_wgt->item(tW_Column,i)->text();

		//'HAVING' clause
		if (tab_wgt->item(7,i) && tab_wgt->item(tW_Having,i)->text()!="")
				having_cl+= (having_cl=="HAVING "? "" : (compact_sql?" AND ":"\n\t\tAND ")) + tab_wgt->item(tW_Column,i)->text() +
							   tab_wgt->item(7,i)->text();

		//'ORDER BY' clause
		if(qobject_cast<QComboBox *>(tab_wgt->cellWidget(tW_Order,i)->children()[1])->currentIndex()>0)
			order_by_cols.insert(make_pair(
				qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,i)->children()[2])->value(), i));
	}

	for(const auto &order : order_by_cols)
	switch(qobject_cast<QComboBox *>(tab_wgt->cellWidget(tW_Order,order.second)->children()[1])->currentIndex())
	{
		case 1:
			order_cl+= (order_cl=="ORDER BY "? "" : (compact_sql?", ":",\n\t\t")) +
								tab_wgt->item(tW_Column,order.second)->text() + " ASC";
			break;
		case 2:
			order_cl+= (order_cl=="ORDER BY "? "" : (compact_sql?", ":",\n\t\t")) +
								tab_wgt->item(tW_Column,order.second)->text() + " DESC";
			break;
		default:
			break;
	}


	//Clean the dust in the optional clauses
	where_cl=="WHERE "      ? where_cl=nullptr  : where_cl+="\n";
	group_cl=="GROUP BY "   ? group_cl=nullptr  : group_cl+="\n";
	having_cl=="HAVING "    ? having_cl=nullptr : having_cl+="\n";
	order_cl=="ORDER BY "   ? order_cl=nullptr  : order_cl+="\n";

	//Agglomerate the result
		result = select_cl +
				 from_cl +
				 where_cl +
				 group_cl +
				 having_cl +
				 order_cl +
				 ";";

	if(!initial_warning)
		msg=nullptr;

	if(msg!=nullptr)
	{
		Messagebox msg_box;
		msg_box.show(msg.toUtf8(), Messagebox::AlertIcon, Messagebox::OkButton);
	}

	return result;
}

QVector < QPair< BaseTable *, QVector < QPair<Column *, Column *> > > >
	GraphicalQueryBuilderCoreWidget::getQueryPath(void)
{
	//The path returned consists of a nested container : first 'FROM' table,
	//and following 'JOIN' clauses, which are a table and its 'ON' statements : a vector of pairs of columns.
	QVector < QPair< BaseTable *, QVector < QPair<Column *, Column *> > > >  result;

	//another stack container to store the pairs of 'ON' statements.
	QVector < QPair<Column *, Column *> > col_vec;

	//temporary-stored JOIN tables during the iteration
	BaseTable *join_tb;

	//Custom-Depth-First-Search heap containers
	visited_vertices.clear();
	disconnected_vertices.clear();
	dfs_rels.clear();
	dfs_result.clear();

	//Set root vertex at first table in the gqb widget :
	//For now the DFS is done from only one root and does not support forests
	//like boost standard DFS does.
	BaseTable * root_vertex = reinterpret_cast<BaseTable *>(
				tab_wgt->item(tW_Table,0)->data(Qt::UserRole).value<void *>());

	updateRequiredVertices();

	// Populate the relation container used for DFS : a vector of pairs (visited flag + rel)
	bool non_supported_rels=false;
	QMap<int, BaseRelationship *> rel_map = gqb_j->getRelPath();
	for(const auto &base_rel:rel_map)
	{
		if( base_rel->getRelTypeAttribute()==Attributes::RelationshipFk ||
			base_rel->getRelTypeAttribute()==Attributes::Relationship11 ||
			base_rel->getRelTypeAttribute()==Attributes::Relationship1n )
		{
				dfs_rels.append(qMakePair(false,base_rel));
		}
		else
			non_supported_rels=true;
	}
	if (non_supported_rels)
		msg+="Relationships with <strong>types</strong> other than fk, 1-to-1 and 1-to-many are "
			 "<strong>discarded</strong> for now!<br/><br/>";

	customDepthFirstSearch(root_vertex);

	for (const auto &vertex:required_vertices)
	{
		if(!visited_vertices.contains(vertex))
			disconnected_vertices.append(vertex);
	}

	QVector <QPair<short, BaseRelationship *>>::iterator dfs_res_itr;
	for(dfs_res_itr=dfs_result.begin();dfs_res_itr!=dfs_result.end(); dfs_res_itr++)
	{
		col_vec.clear();

		if(dfs_res_itr==dfs_result.begin())
		{
			//Add only in first map_itr.value() the first table - this is the 'FROM' part, no 'ON' cols.
			col_vec.push_back(qMakePair(nullptr,nullptr));
			result.push_back(qMakePair(
				root_vertex,
				col_vec));
			col_vec.clear();
		}

		switch(dfs_res_itr->first)
		{
			case DFS_flowRel:
				join_tb= dfs_res_itr->second->getTable(BaseRelationship::DstTable);
				break;
			case DFS_reverseRel:
				join_tb= dfs_res_itr->second->getTable(BaseRelationship::SrcTable);
				break;
			default:
				join_tb=nullptr;
		}

		auto src_cols=dfs_res_itr->second->getReferenceForeignKey()->getColumns(Constraint::SourceCols);
		auto ref_cols=dfs_res_itr->second->getReferenceForeignKey()->getColumns(Constraint::ReferencedCols);
		for(unsigned long i=0;i<src_cols.size();i++)
		{
			col_vec.append(dfs_res_itr->first%2==0?
						(qMakePair(src_cols[i], ref_cols[i])) :
						(qMakePair(ref_cols[i], src_cols[i])));
		}
		result.append(qMakePair(
				join_tb,
				col_vec));
	}
	if(result.empty())
		disconnected_vertices.push_front(root_vertex);

	return result;
}

void GraphicalQueryBuilderCoreWidget::customDepthFirstSearch(BaseTable * current_vertex)
{
	//This function is recursive so it would need to be reentrant=stack-only,
	//but we manipulate data cross-calls :
	//we store meaningful containers in heap (class attributes) not in stack.
	if (visited_vertices.indexOf(current_vertex)!=-1)
	{
		join_in_where=true;
		dfs_result.back().first+=2;
		return;
	}

	visited_vertices.push_back(current_vertex);

	QVector<QPair<bool, BaseRelationship *>>::iterator dfs_rel_itr;
	for (dfs_rel_itr=dfs_rels.begin();dfs_rel_itr!=dfs_rels.end();dfs_rel_itr++)
	{
		if ((dfs_rel_itr->second->getTable(BaseRelationship::SrcTable)==current_vertex ||
			 dfs_rel_itr->second->getTable(BaseRelationship::DstTable)==current_vertex) &&
			 !dfs_rel_itr->first /* this rel has not yet been traversed */)
		{
				dfs_result.append(qMakePair(
						static_cast<short>(dfs_rel_itr->second->getTable(BaseRelationship::SrcTable)!=current_vertex),
						dfs_rel_itr->second));
				dfs_rel_itr->first=true;
				customDepthFirstSearch( (dfs_result.back().first%2==0 ?
								 dfs_rel_itr->second->getTable(BaseRelationship::DstTable) :
								 dfs_rel_itr->second->getTable(BaseRelationship::SrcTable)) );
		}
	}
}

void GraphicalQueryBuilderCoreWidget::initializeColumn(int col_nb, BaseObject *bObj)
{
	QTableWidgetItem *tab_item;
	tab_wgt->insertColumn(col_nb);

	//Initialize select checkbox
	auto * w1 = new QWidget;
	auto *w1_cb=new QCheckBox;
	auto * l1 = new QHBoxLayout(w1);
	l1->addWidget(w1_cb);
	l1->setAlignment(Qt::AlignCenter);
	l1->setContentsMargins(0,0,0,0);
	w1_cb->setToolTip("Include this column in the select clause");
	tab_wgt->setCellWidget(0,col_nb,w1);
	qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Selection,col_nb)->
							  children().last())->setCheckState(Qt::Checked);

	//Initialize group-by
	auto *w2 = new QWidget;
	auto *w2_cb =new QCheckBox;
	auto *l2 = new QHBoxLayout(w2);
	l2->addWidget(w2_cb);
	l2->setAlignment(Qt::AlignCenter);
	l2->setContentsMargins(0,0,0,0);
	w2_cb->setToolTip("Include this column in the group-by clause");
	w2_cb->setCheckState(Qt::Unchecked);
	tab_wgt->setCellWidget(tW_Group,col_nb,w2);

	//Initialize order-by : combobox and spinbox
	auto *w3 = new QWidget;
	auto *w3_cb=new QComboBox;
	auto *w3_sb =new QSpinBox;
	w3->setObjectName("ob_wgt"); //prevents color propagation to children in columnSelectChecked
	w3_sb->setMinimum(1);
	w3_cb->insertItem(0,"");
	w3_cb->insertItem(1,"ASC");
	w3_cb->insertItem(2,"DESC");
	auto *l3 = new QHBoxLayout(w3);
	l3->addWidget(w3_cb);
	l3->addWidget(w3_sb);
	l3->setContentsMargins(0,0,0,0);
	w3_cb->setToolTip("Include this column in the order-by clause");
	w3_sb->hide();
	tab_wgt->setCellWidget(tW_Order,col_nb,w3);

	connect(w3_cb, QOverload<int>::of(&QComboBox::currentIndexChanged),[ &, w3](int index){
		int wgt_col=0;
		for (int col=0;col<tab_wgt->columnCount();col++)
			if(tab_wgt->cellWidget(tW_Order,col)==w3)
				wgt_col=col;
		orderByCountChanged(wgt_col, index);
		tab_wgt->resizeColumnsToContents();
	});

	connect(w3_sb, QOverload<int>::of(&QSpinBox::valueChanged), [&, w3](int new_value){
		int wgt_col=0;
		for (int col=0;col<tab_wgt->columnCount();col++)
			if(tab_wgt->cellWidget(tW_Order,col)==w3)
				wgt_col=col;
		swapOrderBySpins(wgt_col, new_value);
	});

	//Two cases : the item selected is a table/view or a column
	if(bObj->getObjectType()==ObjectType::Table ||
			bObj->getObjectType()==ObjectType::View  )
	{
		//Initialize schema
		tab_item=new QTableWidgetItem;
		tab_item->setText(bObj->getSchema()->getName());
		tab_wgt->setItem(tW_Schema, col_nb, tab_item);

		//Initialize table/view
		tab_item=new QTableWidgetItem;
		tab_item->setText(bObj->getName());
		tab_item->setData(Qt::UserRole, QVariant::fromValue<void *>(
							  dynamic_cast<BaseTable *>(bObj)));
		tab_wgt->setItem(tW_Table, col_nb, tab_item);

		//Initialize column
		tab_item=new QTableWidgetItem;
		tab_item->setText("*");
		tab_wgt->setItem(tW_Column, col_nb, tab_item);

		//Initialize where and having, then disable them + group & order
		tab_item=new QTableWidgetItem;
		tab_wgt->setItem(tW_Where, col_nb, tab_item);
		auto currentFlags = tab_wgt->item(tW_Where,col_nb)->flags();
		tab_wgt->item(tW_Where,col_nb)->setFlags(currentFlags & (~Qt::ItemIsEditable));
		tab_wgt->item(tW_Where,col_nb)->setBackgroundColor(Qt::lightGray);

		tab_item=new QTableWidgetItem;
		tab_wgt->setItem(tW_Having, col_nb, tab_item);
		currentFlags = tab_wgt->item(tW_Having,col_nb)->flags();
		tab_wgt->item(tW_Having,col_nb)->setFlags(currentFlags & (~Qt::ItemIsEditable));
		tab_wgt->item(tW_Having,col_nb)->setBackgroundColor(QColor(195,195,195));

		qobject_cast<QWidget *>(tab_wgt->cellWidget(tW_Group,col_nb))->setEnabled(false);
		qobject_cast<QWidget *>(tab_wgt->cellWidget(tW_Order,col_nb))->setEnabled(false);

		qobject_cast<QWidget *>(tab_wgt->cellWidget(tW_Group,col_nb)->children()[1])->hide();
		tab_wgt->cellWidget(tW_Group,col_nb)->setStyleSheet("background-color:rgb(195,195,195);");
		qobject_cast<QWidget *>(tab_wgt->cellWidget(tW_Order,col_nb)->children()[1])->hide();
		tab_wgt->cellWidget(tW_Order,col_nb)->setStyleSheet("background-color:rgb(195,195,195);");
	}

	else if(bObj->getObjectType()==ObjectType::Column)
	{
		//A column not selected cannot be grouped nor odered.
		connect(w1_cb, &QCheckBox::stateChanged, [&, w1](int state){
			int wgt_col=0;
			for (int col=0;col<tab_wgt->columnCount();col++)
				if(tab_wgt->cellWidget(tW_Selection,col)==w1)
					wgt_col=col;
			columnSelectChecked(wgt_col, state);
		});

		//Initialize schema
		tab_item=new QTableWidgetItem;
		tab_item->setText(dynamic_cast<TableObject *>(bObj)->getParentTable()->getSchema()->getName());
		tab_wgt->setItem(tW_Schema, col_nb, tab_item);

		//Initialize table/view
		tab_item=new QTableWidgetItem;
		tab_item->setText(dynamic_cast<TableObject *>(bObj)->getParentTable()->getName());
		tab_item->setData(Qt::UserRole, QVariant::fromValue<void *>(
								  dynamic_cast<BaseTable *>(
								  dynamic_cast<TableObject *>(bObj)->getParentTable())));
		tab_wgt->setItem(tW_Table, col_nb, tab_item);

		//Initialize column
		tab_item=new QTableWidgetItem;
		tab_item->setText(bObj->getName());
		tab_item->setData(Qt::UserRole, QVariant::fromValue<void *>(
								  dynamic_cast<TableObject *>(bObj)));
		tab_wgt->setItem(tW_Column, col_nb, tab_item);

		//Initialize where and having
		tab_item=new QTableWidgetItem;
		tab_wgt->setItem(tW_Where, col_nb, tab_item);

		tab_item=new QTableWidgetItem;
		tab_wgt->setItem(tW_Having, col_nb, tab_item);

	}

	//Paint the schema, table and column cells in a specific color
	tab_wgt->item(tW_Schema,col_nb)->setBackgroundColor(QColor(225,255,255));
	tab_wgt->item(tW_Table,col_nb)->setBackgroundColor(QColor(225,255,255));
	tab_wgt->item(tW_Column,col_nb)->setBackgroundColor(QColor(225,255,255));

	//Disable editing on schema/table/column
	auto currentFlags = tab_wgt->item(tW_Schema,col_nb)->flags();
	tab_wgt->item(tW_Schema,col_nb)->setFlags(currentFlags & (~Qt::ItemIsEditable));
	currentFlags = tab_wgt->item(tW_Table,col_nb)->flags();
	tab_wgt->item(tW_Table,col_nb)->setFlags(currentFlags & (~Qt::ItemIsEditable));
	currentFlags = tab_wgt->item(tW_Column,col_nb)->flags();
	tab_wgt->item(tW_Column,col_nb)->setFlags(currentFlags & (~Qt::ItemIsEditable));

	//Initialize alias, and disable for now
	tab_item=new QTableWidgetItem;
	tab_wgt->setItem(tW_Alias, col_nb, tab_item);
	currentFlags = tab_wgt->item(tW_Alias,col_nb)->flags();
	tab_wgt->item(tW_Alias,col_nb)->setFlags(currentFlags & (~Qt::ItemIsEditable));
}

void GraphicalQueryBuilderCoreWidget::rearrangeTabSections(int log, int oldV, int newV)
{
	//Avoid "unused variable" compiler warnings... compiler attribute [[maybe_unused]] not yet supported.
	log+=1;
	/*
	 * The virtual functions QAbstractItemModel::moveRow and moveColumn are currently not implemented by Qt
	 * in convenience classes like QTableWidget. Ticket ongoing : https://bugreports.qt.io/browse/QTBUG-74013.
	 * For now, we manually propagate a column move, in the view (sectionsMovable), to the model.
	*/
	BaseObject *bObjTmp;
	if(tab_wgt->item(tW_Column,oldV)->text()=="*")
	{
		bObjTmp= dynamic_cast<BaseObject *>(
				reinterpret_cast<BaseTable *>(
						tab_wgt->item(tW_Table,oldV)->data(Qt::UserRole).value<void *>()));
		this->initializeColumn(oldV<newV?newV+1:newV, bObjTmp);
	}
	else
	{
		bObjTmp= dynamic_cast<BaseObject *>(
				reinterpret_cast<TableObject *>(
						tab_wgt->item(tW_Column,oldV)->data(Qt::UserRole).value<void *>()));
		this->initializeColumn(oldV<newV?newV+1:newV, bObjTmp);

		//The column has been initialized : now copy data from old to new.
		for(auto rowN:{tW_Alias, tW_Where, tW_Having})
			tab_wgt->item(rowN, oldV<newV?newV+1:newV)->setText(tab_wgt->item(rowN,oldV<newV?oldV:oldV+1)->text());

		qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Selection,oldV<newV?newV+1:newV)->children().last())->setCheckState(
			qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Selection,oldV<newV?oldV:oldV+1)->children().last())->checkState());

		auto new_group_cb = qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Group,oldV<newV?newV+1:newV)->children()[1]);
		auto old_group_cb = qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Group,oldV<newV?oldV:oldV+1)->children()[1]);
		new_group_cb->blockSignals(true);
		new_group_cb->setCheckState(old_group_cb->checkState());
		new_group_cb->blockSignals(false);

		auto new_order_cb = qobject_cast<QComboBox *>(tab_wgt->cellWidget(tW_Order,oldV<newV?newV+1:newV)->children()[1]);
		auto old_order_cb = qobject_cast<QComboBox *>(tab_wgt->cellWidget(tW_Order,oldV<newV?oldV:oldV+1)->children()[1]);
		new_order_cb->blockSignals(true);
		new_order_cb->setCurrentIndex(old_order_cb->currentIndex());
		new_order_cb->blockSignals(false);

		auto new_order_sb = qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,oldV<newV?newV+1:newV)->children()[2]);
		auto old_order_sb = qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,oldV<newV?oldV:oldV+1)->children()[2]);
		new_order_sb->blockSignals(true);
		new_order_sb->setValue(old_order_sb->value());
		new_order_sb->setVisible(old_order_sb->isVisible());
		new_order_sb->setMaximum(old_order_sb->maximum());
		new_order_sb->blockSignals(false);
	}

	for(auto child:tab_wgt->cellWidget(tW_Order,oldV<newV?oldV:oldV+1)->children())
		child->blockSignals(true);
	this->tab_wgt->removeColumn(oldV<newV?oldV:oldV+1);

	this->tab_wgt->resizeColumnsToContents();
}

void GraphicalQueryBuilderCoreWidget::gqbPathWidgetToggled(bool change)
{
	if(path_btn->isChecked()!=change)
		path_btn->setChecked(change);
}

void GraphicalQueryBuilderCoreWidget::updateRelLabel(void)
{
	auto path_mode_set= gqb_j->path_mode_set;
	QString mode;
	int rel_cnt=0;

	if(path_mode_set.first==GraphicalQueryBuilderPathWidget::Manual && gqb_j->manual_path_tw->rowCount()>0)
	{
		mode="Man: ";
		rel_cnt=gqb_j->manual_path_tw->rowCount();
	}

	else if(path_mode_set.first==GraphicalQueryBuilderPathWidget::Automatic && gqb_j->auto_path_tw->rowCount()>0)
	{
		mode=tr("Auto%1: ").arg(path_mode_set.second+1);
		rel_cnt=dynamic_cast<QTreeWidget *>(
			gqb_j->auto_path_tw->cellWidget(path_mode_set.second,0))->topLevelItem(0)->childCount();
	}

	if(rel_cnt==0)
		rel_cnt_lbl->setVisible(false);
	else
	{
		rel_cnt_lbl->setVisible(true);

		if(rel_cnt==1)
			rel_cnt_lbl->setText(mode+tr("<strong>%1</strong> rel").arg(rel_cnt));
		else
			rel_cnt_lbl->setText(mode+tr("<strong>%1</strong> rels").arg(rel_cnt));
	}
}

void GraphicalQueryBuilderCoreWidget::selectAllItemsFromQuery(void)
{
	QList<BaseObjectView *> query_items;
	model_wgt->getObjectsScene()->clearSelection();

	for(int col=0; col<tab_wgt->columnCount();col++)
	{

			auto item = dynamic_cast<BaseObjectView *>(
					reinterpret_cast<BaseGraphicObject *>(tab_wgt->item(tW_Table,col)->data(Qt::UserRole).value<void *>())
					->getOverlyingObject());
			item->setSelected(true);
			query_items.push_back(item);
	}

	auto rel_path=gqb_j->getRelPath();
	for(const auto &rel:rel_path)
	{
		auto item = dynamic_cast<BaseObjectView *>(rel->getOverlyingObject());
		item->setSelected(true);
		query_items.push_back(item);
	}

	emit s_adjustViewportToItems(query_items);
}

void GraphicalQueryBuilderCoreWidget::highlightQueryColumn(int col)
{
	BaseTableView * tab_v;

	model_wgt->getObjectsScene()->clearSelection();

	tab_v = dynamic_cast<BaseTableView *>(
			reinterpret_cast<BaseGraphicObject *>(tab_wgt->item(tW_Table,col)->data(Qt::UserRole).value<void *>())
			->getOverlyingObject());
	tab_v->setSelected(true);

	model_wgt->getViewport()->centerOn(tab_v);
}


//Set visibility of order-by spinboxes, and if needed,
//call to configure/reconfigure spinboxes further
void GraphicalQueryBuilderCoreWidget::orderByCountChanged(int ob_col, int state)
{
	vector<int> ob_cols;
	for(int col=0; col<tab_wgt->columnCount(); col++)
		if(tW_Order==tW_Group ?
				qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[1])->isChecked() :
				qobject_cast<QComboBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[1])->currentIndex()!=0)
			ob_cols.push_back(col);


	if(state>0 &&ob_cols.size()>1)
			for(const auto &col:ob_cols)
				qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[2])->setVisible(true);
	else if(state==0)
	{
		qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,ob_col)->children()[2])->setVisible(false);
		if(ob_cols.size()==1)
			qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,ob_cols.front())->children()[2])->setVisible(false);
	}
	else if(state<0 and ob_cols.size()==1)
		qobject_cast<QSpinBox *>(
			tab_wgt->cellWidget(tW_Order,ob_cols.front())->children()[2])->setVisible(false);

	if(!ob_cols.empty())
		configureOrderBySpinBoxes(ob_cols, ob_col, state);
}

void GraphicalQueryBuilderCoreWidget::configureOrderBySpinBoxes(vector<int> ob_cols, int ob_col, int state)
{
	if(state>0)
	{
		qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,ob_col)->children()[2])->blockSignals(true);
		qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,ob_col)->children()[2])->setMaximum(ob_cols.size());
		qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,ob_col)->children()[2])->setValue(ob_cols.size());
		qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,ob_col)->children()[2])->blockSignals(false);
	}
	else
	{
		//map - key : value of the spinbox, value : number of the column
		map<int, int> spin_values;
		for(const auto &col:ob_cols)
			spin_values.insert(std::pair<int, int>(
				qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[2])->value(), col));

		int deleted_value = state<0 ? ob_col :
				qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,ob_col)->children()[2])->value();

		for(auto itr=spin_values.upper_bound(deleted_value); itr!=spin_values.end()--;++itr)
		{
			qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,itr->second)->children()[2])->blockSignals(true);
			qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,itr->second)->children()[2])
					->setValue(itr->first-1);
			qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,itr->second)->children()[2])->blockSignals(false);
		}
	}

	for(const auto &col:ob_cols)
	{
		qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[2])->blockSignals(true);
		qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[2])->setMaximum(ob_cols.size());
		qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[2])->blockSignals(false);
	}
}

void GraphicalQueryBuilderCoreWidget::swapOrderBySpins(int ob_col, int new_value)
{
	vector<int> ob_cols;
	for(int col=0; col<tab_wgt->columnCount(); col++)
		if(col!=ob_col &&
					qobject_cast<QComboBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[1])->currentIndex()!=0)
				ob_cols.push_back(col);

	map<int, int> spin_values;
	for(const auto &col:ob_cols)
	{
		spin_values.insert(std::pair<int, int>(
			qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[2])->value(), col));
	}

	int spin_value_to_swap=1;
	for(auto & spin_value : spin_values)
	{
		if(spin_value.first!=spin_value_to_swap)
			continue;
		spin_value_to_swap+=1;
	}
	int col_to_swap=spin_values.find(new_value)->second;

	qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,col_to_swap)->children()[2])->blockSignals(true);
	qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,col_to_swap)->children()[2])->setValue(spin_value_to_swap);
	qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Order,col_to_swap)->children()[2])->blockSignals(false);
}

void GraphicalQueryBuilderCoreWidget::columnSelectChecked(int col, int state)
{
	if(state==Qt::Checked)
	{
		qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Group,col)->children()[1])->setVisible(true);
		tab_wgt->cellWidget(tW_Group,col)->setStyleSheet("background-color:rgb(255,255,255);");
		tab_wgt->item(tW_Having,col)->setFlags(tab_wgt->item(tW_Having,col)->flags() | Qt::ItemIsEditable);
		tab_wgt->item(tW_Having,col)->setBackgroundColor(Qt::white);
		qobject_cast<QComboBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[1])->setVisible(true);
		tab_wgt->cellWidget(tW_Order,col)->setStyleSheet("QWidget#ob_wgt { background-color:rgb(255,255,255) }");
	}
	else if (state==Qt::Unchecked)
	{
		qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Group,col)->children()[1])->setChecked(Qt::Unchecked);
		qobject_cast<QCheckBox *>(tab_wgt->cellWidget(tW_Group,col)->children()[1])->setVisible(false);
		qobject_cast<QSpinBox *>(tab_wgt->cellWidget(tW_Group,col)->children()[2])->setVisible(false);
		tab_wgt->cellWidget(tW_Group,col)->setStyleSheet("background-color:rgb(195,195,195);");

		tab_wgt->item(tW_Having,col)->setFlags(tab_wgt->item(tW_Having,col)->flags() & ~Qt::ItemIsEditable);
		tab_wgt->item(tW_Having,col)->setBackgroundColor(QColor(195,195,195));
		qobject_cast<QComboBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[1])->setCurrentIndex(0);
		qobject_cast<QComboBox *>(tab_wgt->cellWidget(tW_Order,col)->children()[1])->setVisible(false);
		tab_wgt->cellWidget(tW_Order,col)->setStyleSheet("QWidget#ob_wgt { background-color:rgb(195,195,195) }");
	}
}

void GraphicalQueryBuilderCoreWidget::updateRequiredVertices(void)
{
	required_vertices.clear();

	//Populate the list of required (selected - graphically not in tablewidget) vertices (tables)
	bool already_in;
	for (int i=0; i<tab_wgt->columnCount();i++)
	{
		already_in=false;
		for (auto const &req_item: required_vertices)
		{
			if(req_item==reinterpret_cast<BaseTable *>(tab_wgt->item(tW_Table,i)->data(Qt::UserRole).value<void *>()))
			{
					already_in=true;
					break;
			}
		}
		if(!already_in)
		{
			required_vertices.push_back(reinterpret_cast<BaseTable *>(
											tab_wgt->item(tW_Table,i)->data(Qt::UserRole).value<void *>()));
		}
	}
}

QVector <BaseTable *> GraphicalQueryBuilderCoreWidget::getRequiredVertices(void)
{
	updateRequiredVertices();
	return required_vertices;
}

tuple<QHash<BaseTable*, int> ,vector<QPair<int, int>>,
	QHash<QPair<int, int>, QPair<BaseRelationship*, int>>>
		GraphicalQueryBuilderCoreWidget::getConnectedComponents(void)
{
	//Custom-Depth-First-Search heap containers
	required_vertices.clear();
	visited_vertices.clear();
	disconnected_vertices.clear();
	dfs_rels.clear();
	dfs_result.clear();

	for(const auto &tab : *model_wgt->getDatabaseModel()->getObjectList(ObjectType::Table))
		required_vertices.push_back(dynamic_cast<BaseTable *>(tab));

	//Set root vertex at first table in the gqb widget :
	//For now the DFS is done from only one root and does not support forests
	//like boost standard DFS does.
	BaseTable * root_vertex = reinterpret_cast<BaseTable *>(
				tab_wgt->item(tW_Table,0)->data(Qt::UserRole).value<void *>());

	// Populate the relation container used for DFS : a vector of pairs (visited flag + rel)
	for(const auto &base_obj : *model_wgt->getDatabaseModel()->getObjectList(ObjectType::BaseRelationship))
	{
		auto base_rel=dynamic_cast<BaseRelationship*>(base_obj);
		if(	(base_rel->getRelTypeAttribute()==Attributes::RelationshipFk ||
			base_rel->getRelTypeAttribute()==Attributes::Relationship11 ||
			base_rel->getRelTypeAttribute()==Attributes::Relationship1n) &&
				(!gqb_j->vis_only_cb->isChecked() ||
				dynamic_cast<BaseObjectView *>(
				base_rel->getOverlyingObject())->isVisible()))
		{
				dfs_rels.append(qMakePair(false,base_rel));
		}
	}

	customDepthFirstSearch(root_vertex);

	for (const auto &vertex:required_vertices)
	{
		if(!visited_vertices.contains(vertex))
			disconnected_vertices.append(vertex);
	}

	//See QueryBuilderPathWidget::findPath for detail about these containers
	QHash<BaseTable*, int> result_first;
	vector<QPair<int, int>> result_second;
	QHash<QPair<int, int>, QPair<BaseRelationship*, int>> result_third;
	int i=0;
	for(const auto &vertex:required_vertices)
		if(!disconnected_vertices.contains(vertex)) result_first.insert(vertex, i++);

	for(const auto &connected_rel:dfs_result)
	{
		auto edge = qMakePair<int,int>(
					result_first.value(connected_rel.second->getTable(BaseRelationship::SrcTable)),
					result_first.value(connected_rel.second->getTable(BaseRelationship::DstTable)));
		auto edge_reversed = qMakePair<int,int>(
					result_first.value(connected_rel.second->getTable(BaseRelationship::DstTable)),
					result_first.value(connected_rel.second->getTable(BaseRelationship::SrcTable)));
		bool already_in=false;
		for(const auto &existing_edge:result_second)
		{
			if(edge==existing_edge)
			{
				already_in=true;
				break;
			}
		}

		if(!already_in)
		{
			result_second.push_back(edge);
			result_third.insert(edge, qMakePair(connected_rel.second, 1));
			result_third.insert(edge_reversed, qMakePair(connected_rel.second, 1));
		}
	}


	return forward_as_tuple(result_first,result_second, result_third);
}


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

#include "graphicalquerybuilderpathwidget.h"
#include "graphicalquerybuildercorewidget.h"

GraphicalQueryBuilderPathWidget::GraphicalQueryBuilderPathWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	manual_path_tw->setAccessibleName("manual_path_tw");

	connect(hide_tb, SIGNAL(clicked(void)), this, SLOT(hide(void)));

	this->setModel(nullptr);
	this->installEventFilter(this);

	manual_path_tw->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	this->manual_path_tw->verticalHeader()->setSectionsMovable(true);

	path_mode_set=qMakePair<int,int>(Manual,0);
	man_tb->setChecked(true);
	find_paths_tb->setVisible(false);
	options_tb->setVisible(false);
	status_tb->setVisible(false);


	man_tb->setVisible(false);
	auto_tb->setVisible(false);
	reset_tb->setVisible(false);
	set_path_tb->setVisible(false);

	connect(manual_path_tw, &QTableWidget::itemDoubleClicked, [&](QTableWidgetItem *item){
		model_wgt->getObjectsScene()->clearSelection();
		QList<BaseObjectView *> obj;
		obj.push_back(
			dynamic_cast<BaseObjectView *>(
				reinterpret_cast<BaseRelationship *>(
					manual_path_tw->item(item->row(),0)->data(Qt::UserRole).value<void *>())
						->getOverlyingObject()));
		for(const auto &ob:obj)
			ob->setSelected(true);
		//model_wgt->getViewport()->centerOn(dynamic_cast<RelationshipView *>(obj.front())->getLabel(0));
	});


#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
	join_solver_thread=nullptr;
	man_tb->setVisible(true);
	auto_tb->setVisible(true);
	reset_tb->setVisible(true);
	set_path_tb->setVisible(true);

		connect(man_tb, &QToolButton::toggled, [&](bool change){
			if(change)
			{
				path_sw->setCurrentIndex(Manual);
				auto_tb->setChecked(false);
				find_paths_tb->setVisible(false);
				options_tb->setChecked(false);
				options_tb->setVisible(false);
				status_tb->setChecked(false);
				status_tb->setVisible(false);
				set_path_tb->setVisible(true);
			}
		});

		connect(auto_tb, &QToolButton::toggled, [&](bool change){
		if(change)
		{
			path_sw->setCurrentIndex(Automatic);
			man_tb->setChecked(false);
			find_paths_tb->setVisible(true);
            options_tb->setVisible(true);
			options_tb->setChecked(false);
			status_tb->setChecked(false);
			status_tb->setVisible(true);
			set_path_tb->setVisible(true);
		}
	});

	connect(options_tb, &QToolButton::toggled, [&](bool change){
		if(change)
		{
			path_sw->setCurrentIndex(Parameters);
			auto_tb->setChecked(false);
			status_tb->setChecked(false);
			set_path_tb->setVisible(false);
			find_paths_tb->setVisible(true);
		}
	});

	connect(status_tb, &QToolButton::toggled, [&](bool change){
		if(change)
		{
			path_sw->setCurrentIndex(SolverStatus);
			auto_tb->setChecked(false);
			options_tb->setChecked(false);
			set_path_tb->setVisible(false);
			find_paths_tb->setVisible(false);
		}
	});

	connect(set_path_tb, &QToolButton::clicked, [&](){
		if(path_sw->currentIndex()==Manual)
		{
			path_mode_set=qMakePair<int,int>(Manual,0);
			gqb_c->updateRelLabel();
		}

		else if(path_sw->currentIndex()==Automatic && auto_path_tw->currentRow()!=-1)
		{
			path_mode_set=qMakePair<int,int>(Automatic, auto_path_tw->currentRow());
			gqb_c->updateRelLabel();
		}
		else if(path_sw->currentIndex()==Automatic && auto_path_tw->currentRow()==-1 &&
					auto_path_tw->rowCount()>0)
		{
			path_mode_set=qMakePair<int,int>(Automatic, 0);
			gqb_c->updateRelLabel();
		}
	});

	connect(find_paths_tb, &QToolButton::clicked, [&](){
		if(gqb_c->tab_wgt->columnCount()<=1) return;
		options_tb->setChecked(false);
		//status_tb->setChecked(false);   TODO rework : SIGNAL path_sw indexchanged -> manage sw buttons states
		//auto_tb->setChecked(false);
		runSQLJoinSolver();
	});

	connect(stop_solver_pb, &QPushButton::clicked, [&](){
		emit s_stopJoinSolverRequested();
	});

	connect(exact_cb, &QCheckBox::clicked, [&](bool clicked){
		sp_max_cost_sb->setEnabled(!clicked);
		st_limit_sb->setEnabled(!clicked);
	});

	reset_menu.addAction(tr("All"), this, SLOT(resetPaths()));
	reset_menu.addAction(tr("Manual"), this, SLOT(resetPaths()));
	reset_menu.addAction(tr("Automatic"), this, SLOT(resetPaths()));
	reset_menu.addAction(tr("Parameters"), this, SLOT(resetPaths()));
	reset_tb->setMenu(&reset_menu);

	connect(add_custom_cost_tb, &QToolButton::clicked, [&](){
		int r=custom_costs_tw->rowCount();
		custom_costs_tw->insertRow(r);

		auto cb= new QComboBox;
		cb->addItems({"Rel","Constraint","Table","Schema"});
		custom_costs_tw->setCellWidget(r,0,cb);

		auto cb2= new QComboBox;
		cb2->addItems({"Name","Comment"});
		custom_costs_tw->setCellWidget(r,1,cb2);
	});

	connect(reset_custom_costs_tb, &QToolButton::clicked, [&](){
		custom_costs_tw->setRowCount(0);
	});

	connect(auto_path_tw, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(automaticPathSelected(int, int, int, int)));
#endif
}

bool GraphicalQueryBuilderPathWidget::eventFilter(QObject *object, QEvent *event)
{
	auto *k_event=dynamic_cast<QKeyEvent *>(event);

	if (event->type() == QEvent::Hide)
		emit s_visibilityChanged(false);

	else if (event->type() == QEvent::Show)
		emit s_visibilityChanged(true);

	else if(k_event!=nullptr && k_event->key()==Qt::Key_Delete)
	{
		QMap<int, int> ord_sel_rows;
		for(auto sel_item:manual_path_tw->selectedItems())
			ord_sel_rows.insert(manual_path_tw->row(sel_item), manual_path_tw->row(sel_item));
		QMapIterator<int, int> ord_sel_rows_itr(ord_sel_rows);
		ord_sel_rows_itr.toBack();
		while (ord_sel_rows_itr.hasPrevious())
		{
			ord_sel_rows_itr.previous();
			manual_path_tw->removeRow(ord_sel_rows_itr.value());
		}

		gqb_c->updateRelLabel();
		return true;
	}

	return(QWidget::eventFilter(object, event));
}

void GraphicalQueryBuilderPathWidget::setModel(ModelWidget *model_wgt)
{
	bool enable=model_wgt!=nullptr;
	if(this->model_wgt!=nullptr && enable)
		this->resetPaths();

	this->model_wgt=model_wgt;
}

void GraphicalQueryBuilderPathWidget::insertManualRels(QMap<int, BaseObjectView *> q_rels)
{
	for (auto q_rel : q_rels)
	{
		manual_path_tw->insertRow(manual_path_tw->rowCount());
		auto tab_item=new QTableWidgetItem;
		tab_item->setText(q_rel->getUnderlyingObject()->getName());
		tab_item->setData(Qt::UserRole, QVariant::fromValue<void *>(
							dynamic_cast<BaseRelationship *>(q_rel->getUnderlyingObject())));
		manual_path_tw->setItem(manual_path_tw->rowCount()-1, 0, tab_item);
	}
}

QMap<int, BaseRelationship *> GraphicalQueryBuilderPathWidget::getRelPath(void)
{
	QMap<int, BaseRelationship *> rel_path_res;

	if(path_mode_set.first==Manual)
	{
		for(int i=0;i<manual_path_tw->rowCount();i++)
		{
			auto base_rel = reinterpret_cast<BaseRelationship *>(manual_path_tw->item(i,0)->data(Qt::UserRole).value<void *>());
			rel_path_res.insert(i, base_rel);
		}
	}
	else
	{
		auto path_twi=dynamic_cast<QTreeWidget *>(
			auto_path_tw->cellWidget(path_mode_set.second,0))->topLevelItem(0);
		for(int i=0;i<path_twi->childCount();i++)
		{
			auto base_rel = reinterpret_cast<BaseRelationship *>(path_twi->child(i)->data(0,Qt::UserRole).value<void *>());
			rel_path_res.insert(i, base_rel);
		}
	}
	return rel_path_res;
}


#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
void GraphicalQueryBuilderPathWidget::insertAutoRels(paths paths_found)
{
	this->resetAutoPath();

	int i=-1;
	for(auto it=paths_found.begin(); it!=paths_found.end();it++)
	{
		i+=1;
		auto_path_tw->insertRow(auto_path_tw->rowCount());

		//Insert path header : "Path n"
		auto tw= new QTreeWidget;
		tw->setColumnCount(2);
		tw->setHeaderLabels({"Path " + QString::number(i+1),"Cost"});
		auto tw_top_item=new QTreeWidgetItem;
		tw_top_item->setText(0,"Path " + QString::number(i+1));
		tw->addTopLevelItem(tw_top_item);
		auto_path_tw->setCellWidget(auto_path_tw->rowCount()-1,0,tw);

		//Highlight the given path relations if shift modifier pressed.
		connect(tw, &QTreeWidget::itemClicked, [&, tw](QTreeWidgetItem *item, [[maybe_unused]]int column){

			auto k_modifiers = QGuiApplication::queryKeyboardModifiers();
			if(k_modifiers & Qt::ShiftModifier)
			{
				QList<BaseObjectView *> obj;
				if(item==tw->topLevelItem(0))
				{
					model_wgt->getObjectsScene()->clearSelection();
					for(int i=0;i<item->childCount();i++)
					{
						obj.push_back(
							dynamic_cast<BaseObjectView *>(
								reinterpret_cast<BaseRelationship *>(
									item->child(i)->data(0, Qt::UserRole).value<void *>())
										->getOverlyingObject()));
					}
				}
				else
				{
					obj.push_back(
						dynamic_cast<BaseObjectView *>(
							reinterpret_cast<BaseRelationship *>(
								item->data(0, Qt::UserRole).value<void *>())
									->getOverlyingObject()));
				}

				for (const auto &ob:obj)
					ob->setSelected(true);
				emit s_adjustViewportToItems(obj);
			}
		});

		//For this path insert the relations with their weight : "rel_n weight"
		for (const auto &qrel : it.value().second)
		{
			auto tw_item=new QTreeWidgetItem;
			tw_item->setText(0, qrel.first->getName());
			tw_item->setText(1, QString::number(qrel.second));
			tw_item->setData(0,Qt::UserRole, QVariant::fromValue<void *>(qrel.first));
			tw_top_item->addChild(tw_item);
		}
		//Set the total weight of the current path
		tw_top_item->setText(1,QString::number(it.key()));

		tw_top_item->setExpanded(true);
		tw->resizeColumnToContents(0);

		//Set the second column headers
		auto tw2= new QTreeWidget;
		tw2->setColumnCount(1);
		tw2->setHeaderLabels({"Tables"});
		auto_path_tw->setCellWidget(auto_path_tw->rowCount()-1,1,tw2);
		//Insert steiner points (non-terminals that are "central points") with underlined text.
		for (const auto &tab : it.value().first.first)
		{
			auto twi=new QTreeWidgetItem;
			twi->setText(0, tab->getName() + " - " + tab->getComment());
			auto the_font=twi->font(0);
			the_font.setUnderline(true);
			twi->setFont(0,the_font);
			tw2->addTopLevelItem(twi);
		}
		//Insert non-terminals non-steiner.
		for (const auto &tab : it.value().first.second)
		{
			auto twi=new QTreeWidgetItem;
			twi->setText(0, tab->getName() + " - " + tab->getComment());
			tw2->addTopLevelItem(twi);
		}
	}
	auto_path_tw->resizeRowsToContents();
	auto_path_tw->horizontalHeader()->setVisible(true);
	path_sw->setCurrentIndex(1);
}
#endif

void GraphicalQueryBuilderPathWidget::resetPaths(void)
{
	if(!reset_menu.actions().contains(qobject_cast<QAction *>(sender())) ||
			qobject_cast<QAction *>(sender()) == reset_menu.actions().at(0))
	{
		manual_path_tw->setRowCount(0);
		path_sw->setCurrentIndex(0);
		path_mode_set=qMakePair<int,int>(Manual,0);

		auto_path_tw->setRowCount(0);

		gqb_c->updateRelLabel();

		exact_cb->setChecked(true);
		sp_max_cost_sb->setEnabled(false);
		st_limit_sb->setEnabled(false);
		sp_max_cost_sb->setValue(2);
		st_limit_sb->setValue(5);

		vis_only_cb->setChecked(false);
		default_cost_sb->setValue(1);
		cross_sch_cost_sb->setValue(3);

		custom_costs_tw->setRowCount(0);
	}

#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
	else if(qobject_cast<QAction *>(sender()) == reset_menu.actions().at(1))
	{
		manual_path_tw->setRowCount(0);
		path_sw->setCurrentIndex(0);
		path_mode_set=qMakePair<int,int>(Manual,0);
		gqb_c->updateRelLabel();
	}
	else if(qobject_cast<QAction *>(sender()) == reset_menu.actions().at(2))
	{
		auto_path_tw->setRowCount(0);
		path_sw->setCurrentIndex(1);
		path_mode_set=qMakePair<int,int>(Automatic,0);
		gqb_c->updateRelLabel();
	}
	else if(qobject_cast<QAction *>(sender()) == reset_menu.actions().at(3))
	{
		exact_cb->setChecked(true);
		sp_max_cost_sb->setEnabled(false);
		st_limit_sb->setEnabled(false);
		sp_max_cost_sb->setValue(2);
		st_limit_sb->setValue(5);

		vis_only_cb->setChecked(false);
		default_cost_sb->setValue(1);
		cross_sch_cost_sb->setValue(3);

		custom_costs_tw->setRowCount(0);

		real_time_rendering_cb->setChecked(false);
		rt_render_delay_sb->setValue(0);

		resetJoinSolverStatus();
		destroyThread(true);

	}
#endif
}

#ifdef GRAPHICAL_QUERY_BUILDER_JOIN_SOLVER
void GraphicalQueryBuilderPathWidget::automaticPathSelected(int new_row, int new_column, int old_row, int old_column)
{
	auto dummy=new_row+new_column+old_column;
	dummy++;

	if(old_row!=-1 && new_row!=-1)
		{
			auto old_tw1= dynamic_cast<QTreeWidget *>(auto_path_tw->cellWidget(old_row, 0));
			old_tw1->setCurrentItem(nullptr);
			auto old_tw2= dynamic_cast<QTreeWidget *>(auto_path_tw->cellWidget(old_row, 1));
			old_tw2->setCurrentItem(nullptr);
		}
}

void GraphicalQueryBuilderPathWidget::createThread(void)
{
	if(!join_solver_thread)
	{
		join_solver_thread=new QThread(this);
		join_solver=new GraphicalQueryBuilderJoinSolver(
					this, join_solver_thread, real_time_rendering_cb->isChecked(), rt_render_delay_sb->value());
		join_solver->moveToThread(join_solver_thread);

		connect(join_solver_thread, &QThread::started, [&](){
			path_sw->setCurrentIndex(SolverStatus);
			join_solver_status_wgt->show();
			join_solver->findPaths();
		});

		connect(this, SIGNAL(s_stopJoinSolverRequested()), join_solver, SLOT(handleJoinSolverStopRequest()),
				Qt::QueuedConnection);

		connect(join_solver_thread, &QThread::finished, [&](){
			join_solver_thread=nullptr;
		});

		qRegisterMetaType<paths>();
		connect(join_solver,
				SIGNAL(s_pathsFound(paths)),
				this,
				SLOT(handlePathsFound(paths)),
				Qt::QueuedConnection);

		//TODO those queued messages are handled poorly in VMs. Would need bottlenecking-checks.
		connect(join_solver,
				SIGNAL(s_progressUpdated(short,
										 short,short,long long,int,
										 int,int,long long,
										 int,long long,long long,long long)),
				this,
				SLOT(updateProgress(short,
									short,short,long long,int,
									int,int,long long,
									int,long long,long long,long long)),
				Qt::QueuedConnection);

		connect(join_solver, SIGNAL(s_solverStopped()), this, SLOT(stopSolver()), Qt::QueuedConnection);

		qRegisterMetaType<bts>();
		connect(join_solver, SIGNAL(s_progressTables(int, bts)),
				this, SLOT(progressTables(int, bts)), Qt::QueuedConnection);
		connect(join_solver_thread, &QThread::finished, [&](){progressTables(10,{nullptr});});
	}
}

void GraphicalQueryBuilderPathWidget::handlePathsFound(paths p)
{
	if(III_prb->isEnabled())
		III_prb->setValue(100);
	insertAutoRels(p);
	destroyThread(true);
	stop_solver_pb->setEnabled(false);
	auto_tb->toggle();
}


void GraphicalQueryBuilderPathWidget::destroyThread(bool force)
{
	if(join_solver_thread && force)
	{
		disconnect(join_solver_thread, &QThread::started, nullptr, nullptr);
		disconnect(this, SIGNAL(s_stopJoinSolverRequested()), nullptr, nullptr);
		disconnect(stop_solver_pb, &QPushButton::toggled, nullptr, nullptr);
		disconnect(join_solver, SIGNAL(s_pathsFound(paths)), nullptr, nullptr);
		disconnect(join_solver,
				SIGNAL(s_progressUpdated(short,
										 short,short,long long,int,
										 int,int,long long,
										 int,long long,long long,long long)),
				nullptr, nullptr);
		disconnect(join_solver, SIGNAL(s_solverStopped()), nullptr, nullptr);

		delete(join_solver);
		join_solver_thread->quit();
	}
}

void GraphicalQueryBuilderPathWidget::runSQLJoinSolver(void)
{
	createThread();
	resetJoinSolverStatus();
	stop_solver_pb->setEnabled(true);
	join_solver_thread->start();
}

void GraphicalQueryBuilderPathWidget::updateProgress(
		short mode,
		short st_round, short powN, long long st_comb, int st_found,
		int sp_current, int sp_current_on, long long sp_found,
		int st_fround, long long mult_entry, long long mult_entry_on, long long paths_found)
{
	if(!join_solver_thread || !join_solver_thread->isRunning() || join_solver->stop_solver_requested)
		return;

	QLocale big_nb_locale;
	big_nb_locale.setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

	//static constexpr unsigned
	//	Progress_ShortPathMod0=0,
	//	Progress_SteinerRound=1,
	//	Progress_SteinerComb=2,
	//	Progress_SuperEdgeRound=3,
	//	Progress_ShortPathMod1=4,
	//	Progress_FinalRound1=5,
	//	Progress_FinalRound2=6,
	//	Progress_FinalRound3=7,
	//	Progress_FinalRound4=8;

	switch(mode)
	{
	case GraphicalQueryBuilderJoinSolver::Progress_ShortPathMod0: //Two tables to join
		st_round_lbl->setEnabled(false);
		powN_lbl->setEnabled(false);
		st_comb_lbl->setEnabled(false);
		st_comb_on_lbl->setEnabled(false);
		st_found_lbl->setEnabled(false);
		st_found_on_lbl->setEnabled(false);
		I_prb->setEnabled(false);
		sp_current_lbl->setText("1");
		sp_current_on_lbl->setText("1");
		II_prb->setEnabled(false);
		sp_found_lbl->setText(
					big_nb_locale.toString(sp_found));
		st_fround_lbl->setEnabled(false);
		st_fround_on_lbl->setEnabled(false);
		III_prb->setEnabled(false);
		paths_found_lbl->setText(
					big_nb_locale.toString(paths_found));
		break;

	case GraphicalQueryBuilderJoinSolver::Progress_SteinerRound: //k+1-Steiner round
		st_round_lbl->setText(QString::number(st_round));
		powN_lbl->setText(QString::number(powN));
		st_comb_lbl->setText(
					big_nb_locale.toString(st_comb));
		st_comb_on_lbl->setText(
					big_nb_locale.toString((long long) pow(2,powN)) +
					" (2^" + QString::number(powN) +")");
		if(st_found_on_lbl->text()=="/") st_found_on_lbl->setText("/ "+QString::number(st_limit_sb->value()));
		break;

	case GraphicalQueryBuilderJoinSolver::Progress_SteinerComb: //k+1-Steiner combination
		st_comb_lbl->setText(QString::number(st_comb));
		st_found_lbl->setText(QString::number(st_found));
		I_prb->setValue(st_found*100/st_limit_sb->value());
		break;

	case GraphicalQueryBuilderJoinSolver::Progress_SuperEdgeRound: //super_edge round
		I_prb->setValue(100);
		sp_current_lbl->setText(QString::number(sp_current));
		sp_current_on_lbl->setText("/ "+QString::number(sp_current_on));
		II_prb->setValue(
					(sp_current<1?0:sp_current-1)*100/sp_current_on);
		break;

	case GraphicalQueryBuilderJoinSolver::Progress_ShortPathMod1: // sub-paths found
		sp_found_lbl->setText(
					big_nb_locale.toString((long long)sp_found));
		break;

	case GraphicalQueryBuilderJoinSolver::Progress_FinalRound1: //multiplication a
		II_prb->setValue(100);
		st_fround_lbl->setText(QString::number(st_fround));
		if(st_fround_on_lbl->text()=="/") st_fround_on_lbl->setText("/ "+st_found_lbl->text());
		III_prb->setValue(
					(st_fround<1?0:st_fround-1)*100/st_found_lbl->text().toInt());
		break;

	case GraphicalQueryBuilderJoinSolver::Progress_FinalRound2: // multiplication b
		mult_entry_on_lbl->setText(
					big_nb_locale.toString((long long)mult_entry_on));
		break;

	case GraphicalQueryBuilderJoinSolver::Progress_FinalRound3: // multiplication c
		mult_entry_lbl->setText(
					big_nb_locale.toString((long long)mult_entry));
		break;

	case GraphicalQueryBuilderJoinSolver::Progress_FinalRound4: //multiplication d
		paths_found_lbl->setText(
					big_nb_locale.toString((long long)paths_found));
	}
}

void GraphicalQueryBuilderPathWidget::resetJoinSolverStatus(void){
	st_round_lbl->setEnabled(true);
	st_round_lbl->setText("");

	powN_lbl->setEnabled(true);
	powN_lbl->setText("");

	st_comb_lbl->setEnabled(true);
	st_comb_lbl->setText("");

	st_comb_on_lbl->setEnabled(true);
	st_comb_on_lbl->setText("/");

	st_found_lbl->setEnabled(true);
	st_found_lbl->setText("");

	st_found_on_lbl->setEnabled(true);
	st_found_on_lbl->setText("/");

	I_prb->setEnabled(true);
	I_prb->setValue(0);

	sp_current_lbl->setText("");
	sp_current_on_lbl->setText("/");

	II_prb->setEnabled(true);
	II_prb->setValue(0);

	sp_found_lbl->setText("");

	st_fround_lbl->setEnabled(true);
	st_fround_lbl->setText("");

	st_fround_on_lbl->setEnabled(true);
	st_fround_on_lbl->setText("/");

	mult_entry_lbl->setEnabled(true);
	mult_entry_lbl->setText("");

	mult_entry_on_lbl->setEnabled(true);
	mult_entry_on_lbl->setText("/");

	III_prb->setEnabled(true);
	III_prb->setValue(0);

	paths_found_lbl->setText("");
}

void GraphicalQueryBuilderPathWidget::stopSolver(){
	destroyThread(true);
	stop_solver_pb->setEnabled(false);
}

void GraphicalQueryBuilderPathWidget::progressTables(int mode, bts t)
{
	for(auto &pix:pixs)
	{
		dynamic_cast<QGraphicsScene *>(model_wgt->getObjectsScene())->removeItem(pix);
		delete pix;
	}
	pixs.clear();

	//PT_SR=0,	//Steiner points
	//PT_SP1=1,	//Source and target
	//PT_SP2=2,	//Predecessor map
	//PT_FR1=3,	//Steiner points
	//PT_FR2=4;	//Involved tables non steiner
	//(10 end)
	switch(mode)
	{
	case 0: //Steiner points
		for(auto bt:t)
		{
			auto btv=dynamic_cast<BaseObjectView *>(bt->getOverlyingObject());
			auto rect=QRectF(btv->mapToScene(btv->boundingRect()).boundingRect());
			pixs.push_back(addPix(rect.center(),QColor(50,205,50)));
		}
		break;
	case 1: //Source and target
		for(auto &pix:tr_pixs)
		{
			model_wgt->getObjectsScene()->removeItem(pix);
			delete pix;
		}
		tr_pixs.clear();
		for(auto bt:t)
		{
			auto btv=dynamic_cast<BaseObjectView *>(bt->getOverlyingObject());
			auto rect=QRectF(btv->mapToScene(btv->boundingRect()).boundingRect());
			tr_pixs.push_back(addPix(rect.center(),QColor(255,140,0)));
		}
		break;
	case 2: //Predecessor map
		for(auto bt:t)
		{
			auto btv=dynamic_cast<BaseObjectView *>(bt->getOverlyingObject());
			auto rect=QRectF(btv->mapToScene(btv->boundingRect()).boundingRect());
			pixs.push_back(addPix(rect.center(),Qt::yellow));
		}
		break;
	case 3: //Steiner points
		for(auto &pix:tr_pixs)
		{
			model_wgt->getObjectsScene()->removeItem(pix);
			delete pix;
		}
		tr_pixs.clear();
		for(auto bt:t)
		{
			auto btv=dynamic_cast<BaseObjectView *>(bt->getOverlyingObject());
			auto rect=QRectF(btv->mapToScene(btv->boundingRect()).boundingRect());
			tr_pixs.push_back(addPix(rect.center(),QColor(50,205,50)));
		}
		break;
	case 4: //Involved tables non steiner
		for(auto bt:t)
		{
			auto btv=dynamic_cast<BaseObjectView *>(bt->getOverlyingObject());
			auto rect=QRectF(btv->mapToScene(btv->boundingRect()).boundingRect());
			pixs.push_back(addPix(rect.center(),Qt::green));
		}
		break;
	case 10:
		for(auto &pix:tr_pixs)
		{
			model_wgt->getObjectsScene()->removeItem(pix);
			delete pix;
		}
		tr_pixs.clear();
	}

	model_wgt->getObjectsScene()->update();
}

QGraphicsItem * GraphicalQueryBuilderPathWidget::addPix(QPointF pos, QColor col)
{
	QRadialGradient radialGrad(40, 40, 40);
	radialGrad.setColorAt(0, col);
	radialGrad.setColorAt(0.2, col);
	radialGrad.setColorAt(1, Qt::transparent);
	QPixmap pixmap(80, 80);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setPen(Qt::NoPen);
	painter.setBrush(radialGrad);
	painter.drawEllipse(0, 0, 80, 80);
	painter.end();

	auto zis_pix=model_wgt->getObjectsScene()->addPixmap(pixmap);
	zis_pix->setZValue(40);
	zis_pix->setPos(pos.x()-40,pos.y()-40);
	zis_pix->setVisible(true);
	zis_pix->setEnabled(true);
	zis_pix->setActive(true);
	return zis_pix;
}

#endif

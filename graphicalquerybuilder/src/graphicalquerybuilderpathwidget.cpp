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

#include "pgmodeleruins.h"

#include <boost/range/algorithm/copy.hpp>
#include <iostream>
//#include <typeinfo>
//#include "boost/type_index.hpp"


using EdgeProp = boost::property<boost::edge_weight_t, int>;
typedef boost::adjacency_list<
	boost::vecS, boost::vecS, boost::undirectedS,
	boost::property<boost::vertex_color_t, int>, EdgeProp>  Graph ;
using Edge = QPair<int, int>;
using GraphMT = paal::data_structures::graph_metric<Graph, int>;
using Terminals = std::vector<int>;
using edge_parallel_category = boost::allow_parallel_edge_tag;
using CostMap=paal::data_structures::graph_metric<Graph,
						int, paal::data_structures::graph_type::sparse_tag>;
using Path = QVector<Edge>;


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

	connect(man_tb, &QToolButton::clicked, [&](){
		path_sw->setCurrentIndex(Manual);
		man_tb->setChecked(true);
		auto_tb->setChecked(false);
		find_paths_tb->setVisible(false);
		options_tb->setChecked(false);
		options_tb->setVisible(false);
		set_path_tb->setVisible(true);
	});

	connect(auto_tb, &QToolButton::clicked, [&](){
		path_sw->setCurrentIndex(Automatic);
		auto_tb->setChecked(true);
		man_tb->setChecked(false);
		find_paths_tb->setVisible(true);
		options_tb->setVisible(true);
		options_tb->setChecked(false);
		set_path_tb->setVisible(true);
	});
	
	connect(options_tb, &QToolButton::clicked, [&](){
		path_sw->setCurrentIndex(Parameters);
		auto_tb->setChecked(false);
		set_path_tb->setVisible(false);
		find_paths_tb->setVisible(false);
	});

	connect(set_path_tb, &QToolButton::clicked, [&](){
		if(path_sw->currentIndex()==Manual)
		{
			path_mode_set=qMakePair<int,int>(Manual,0);
			gqb_c->updateRelLabel();
		}
		if(path_sw->currentIndex()==Automatic && auto_path_tw->currentRow()!=-1)
		{
			path_mode_set=qMakePair<int,int>(Automatic, auto_path_tw->currentRow());
			gqb_c->updateRelLabel();
		}
	});

	connect(manual_path_tw, &QTableWidget::itemDoubleClicked, [&](QTableWidgetItem *item){
		model_wgt->getObjectScene()->clearSelection();
		QList<BaseObjectView *> obj;
		obj.push_back(
			dynamic_cast<BaseObjectView *>(
				reinterpret_cast<BaseRelationship *>(
					manual_path_tw->item(item->row(),0)->data(Qt::UserRole).value<void *>())
						->getOverlyingObject()));
		for(const auto &ob:obj)
			ob->setSelected(true);
//		model_wgt->getViewport()->centerOn(dynamic_cast<RelationshipView *>(obj.front())->getLabel(0));
	});

	connect(find_paths_tb, &QToolButton::clicked, [&](){
		if(gqb_c->tab_wgt->columnCount()==0) return;
		auto path=findPath();
		this->resetAutoPath();
		this->insertAutoRels(path);
	});

	connect(exact_cb, &QCheckBox::clicked, [&](bool clicked){
		sp_limit_sb->setEnabled(!clicked);
		st_limit_sb->setEnabled(!clicked);
	});

	reset_menu.addAction(trUtf8("All"), this, SLOT(resetPaths()));
	reset_menu.addAction(trUtf8("Manual"), this, SLOT(resetPaths()));
	reset_menu.addAction(trUtf8("Automatic"), this, SLOT(resetPaths()));
	reset_menu.addAction(trUtf8("Parameters"), this, SLOT(resetPaths()));
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
//	connect(this, SIGNAL(s_automaticPathSelected(int)),this, SLOT(automaticPathSelected(int)));
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

QMultiMap<int,
		  QPair<
					QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
					QVector<QPair<BaseRelationship*, int>
		>>> GraphicalQueryBuilderPathWidget::findPath(void)
{

	QMultiMap<int,
			QPair<
						QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
						QVector<QPair<BaseRelationship*, int>
			>>> super_res;

	QVector<QPair<
				QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
				QVector<QPair<BaseRelationship*, int>>
			>> result={
		qMakePair<QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
		QVector<QPair<BaseRelationship*, int>>>(
		{qMakePair<QVector<BaseTable*>, QVector<BaseTable*>>({nullptr}, {nullptr})},
		{qMakePair<BaseRelationship*, int>(nullptr,0)})};

	QHash<BaseTable*, int> tables;
	QHash<int, BaseTable*> tables_r;
	vector<Edge> edges;
	//Hash table that binds an edge (a pair of integers) representation to a pair :
	// 1 its BaseRelationship and 2 its weight
	QHash<Edge, QPair<BaseRelationship*, int>> edges_hash;

	// I. Detect connected components
	auto return_tuple=gqb_c->getConnectedComponents();
	tables=std::move(get<0>(return_tuple));
	edges=std::move(get<1>(return_tuple));
	edges_hash=std::move(get<2>(return_tuple));

	for (auto it=tables.begin();it!=tables.end();it++)
	{
		tables_r.insert(it.value(), it.key());
	}

//	auto uni=edges_hash.uniqueKeys();
//	for(auto it=edges_hash.begin(); it!= edges_hash.end();it++)
//	{
//		if(uni.contains(it.key()))
//			uni.removeOne(it.key());
//		else
//			uni.push_back(it.key());
//	}
//	for (const auto &un:uni)
//	{
//		auto a=tables_r.value(un.first)->getName();
//		auto b=tables_r.value(un.second)->getName();
//	}

//	for (auto it=tables_r.begin();it!=tables_r.end();it++)
//		qDebug() << it.key() << it.value()->getName();

	gqb_c->updateRequiredVertices();

	int nb_required_vertices_connected=0;
	for(const auto &req_vertex:gqb_c->required_vertices)
		if(!gqb_c->disconnected_vertices.contains(req_vertex)) nb_required_vertices_connected+=1;

//	if(nb_required_vertices_connected<2)
//	{
//		qDebug() << "<2";
//	}

	//Setup relationship weights
	vector<int> weights;

	QList<std::tuple<QString, QString, QString, int>> cost_list;
	if(custom_costs_tw->rowCount()>0)
		for(int i=0; i<custom_costs_tw->rowCount();i++)
		{
			cost_list.push_back(forward_as_tuple(
						dynamic_cast<QComboBox *>(custom_costs_tw->cellWidget(i,0))->currentText(),
						dynamic_cast<QComboBox *>(custom_costs_tw->cellWidget(i,1))->currentText(),
						custom_costs_tw->item(i,2)->text(),
						custom_costs_tw->item(i,3)->text().toInt()));
		}
	QRegExp regexp;
	regexp.setPatternSyntax(QRegExp::Wildcard);

	int weight;
	for(auto edge:edges)
	{
		Edge reversed_edge=qMakePair<int,int>(edge.second, edge.first);

		//Set default cost...
		weight=default_cost_sb->value();

		//...add cross-schema extra cost ...
		if(tables_r.value(edge.first)->getSchema()!=
			tables_r.value(edge.second)->getSchema())
				weight+=cross_sch_cost_sb->value();

		//... and custom extra costs.
		for(const auto &custom_cost:cost_list)
		{
			regexp.setPattern(get<2>(custom_cost));
			auto obj=std::move(get<0>(custom_cost));
			auto att=std::move(get<1>(custom_cost));
			if(obj=="Rel")
			{
				if(att=="Name")
				{
					if(regexp.indexIn(edges_hash.value(edge).first->getName())>=0)
						weight+=get<3>(custom_cost);
				}
				else if(att=="Comment")
				{
					if(regexp.indexIn(edges_hash.value(edge).first->getComment())>=0)
						weight+=get<3>(custom_cost);
				}
			}
			else if(obj=="Constraint")
			{
				if(att=="Name")
				{
					if(regexp.indexIn(edges_hash.value(edge).first->getReferenceForeignKey()->getName())>=0)
						weight+=get<3>(custom_cost);
				}
				else if(att=="Comment")
				{
					if(regexp.indexIn(edges_hash.value(edge).first->getReferenceForeignKey()->getComment())>=0)
						weight+=get<3>(custom_cost);
				}
			}
			else if (obj=="Table")
			{
				if(att=="Name")
				{
					if(regexp.indexIn(tables_r.value(edge.first)->getName())>=0 ||
						regexp.indexIn(tables_r.value(edge.second)->getName())>=0)
							weight+=get<3>(custom_cost);
				}
				else if(att=="Comment")
				{
					if(regexp.indexIn(tables_r.value(edge.first)->getComment())>=0 ||
						regexp.indexIn(tables_r.value(edge.second)->getComment())>=0)
							weight+=get<3>(custom_cost);
				}
			}
			else if(obj=="Schema")
			{
				if(att=="Name")
				{
					if(regexp.indexIn(tables_r.value(edge.first)->getSchema()->getName())>=0 ||
						regexp.indexIn(tables_r.value(edge.second)->getSchema()->getName())>=0)
							weight+=get<3>(custom_cost);
				}
				else if(att=="Comment")
				{
					if(regexp.indexIn(tables_r.value(edge.first)->getSchema()->getComment())>=0 ||
						regexp.indexIn(tables_r.value(edge.second)->getSchema()->getComment())>=0)
							weight+=get<3>(custom_cost);
				}
			}
		}
//		qDebug() << edge << " " << weight;
		weights.push_back(weight);
		for(auto it=edges_hash.begin();it!=edges_hash.end();it++)
			if(it.key()==edge||it.key()==reversed_edge)
				it.value().second=weight;
	}

	//vector<int> weights(edges.size(), 1);

	//Setup the boost graph and its paal metric
	Graph g(edges.begin(), edges.end(), weights.begin(), tables.size());
	auto gm = GraphMT(g);

//	auto boost_edges=boost::edges(g);

//	std::pair<Graph::out_edge_iterator,
//			Graph::out_edge_iterator>
//			es=boost::out_edges(3, g);
//	auto zeweight=boost::get(boost::edge_weight, g);

//	for(auto it=es.first;it!=es.second;it++)
//	{
//		auto zesource=boost::source(*it, g);
//		auto zeother=boost::get(zeweight, it);
//		auto a=0;
//	}

	//II. Setup Dreyfus Wagner terminals and non-terminals
	QVector<int> terminals, nonterminals;
	for(const auto &req_vertex:gqb_c->getRequiredVertices())
		if(!gqb_c->disconnected_vertices.contains(req_vertex)) terminals.push_back(tables.value(req_vertex));
	for(const auto &vertex:tables)
		if(!terminals.contains(vertex))
			nonterminals.push_back(vertex);

	auto dw = paal::make_dreyfus_wagner(gm, terminals, nonterminals);
	auto cost_map = dw.get_cost_map();

	//-------------------------------------------------------------------------------------------------
	//II. a) If only two tables to join...
	if(nb_required_vertices_connected==2)
	{
//		qDebug() << "=2 co:" << nb_required_vertices_connected << " disco:" << gqb_c->required_vertices.size() - nb_required_vertices_connected;
		int start, goal;

		start=terminals[0];
		goal=terminals[1];
		terminals.pop_back();

		auto edge=qMakePair<int, int>(start, goal);
		int cost=cost_map(start, goal);

		int paths_found=0;
		while(paths_found<sp_limit_sb->value())
		{
			auto paths=getDetailedPaths(edge, terminals, cost++, cost_map,tables,edges_hash);
			if(paths.empty()) break;
			for(int i=0;i<paths.size();i++)
			{
				QVector<BaseTable*> tables_res={nullptr};
				if(!exact_cb->isChecked() && paths_found==sp_limit_sb->value()) break;

				if(i>0 || paths_found>0)
					result.insert(paths_found,
						(qMakePair<QPair<QVector<BaseTable*>,QVector<BaseTable*>>, QVector<QPair<BaseRelationship*, int>>>(
							 qMakePair<QVector<BaseTable*>,QVector<BaseTable*>>({nullptr}, {nullptr}),
							 {qMakePair(nullptr,0)})));

				for (const auto &sub_edge:paths[i])
				{
						result[paths_found].second.push_back(edges_hash.value(sub_edge));
						auto tr1=tables_r.value(sub_edge.first);
						if(!tables_res.contains(tr1) && !terminals.contains(sub_edge.first))
						{
							result[paths_found].first.second.push_back(tr1);
							tables_res.push_back(tr1);
						}
						auto tr2=tables_r.value(sub_edge.second);
						if(!tables_res.contains(tr2) && terminals.contains(sub_edge.second))
						{
							result[paths_found].first.second.push_back(tr2);
							tables_res.push_back(tr2);
						}
				}

				paths_found++;
			}
			if(exact_cb->isChecked()) break;
		}
	}

	//-------------------------------------------------------------------------------------------------
	//II. b) ...if 3 or more tables to join,
	//run Optimal-Dreyfus-Wagner algorithm, followed by Dijkstra to find edge detail.
	else if(nb_required_vertices_connected>2)
	{
//		qDebug() << ">2 co:" << nb_required_vertices_connected << " disco:" << gqb_c->required_vertices.size() - nb_required_vertices_connected;
		QMap< QPair< QVector<int>, QVector<Edge> >, int > dw_results;
		QVector<int> dw_subresult1;
		QVector<Edge> dw_subresult2;

		dw.solve();

		// print result
//		std::cout << "Cost = " << dw.get_cost() << std::endl;
		std::cout << "Steiner points:" << std::endl;
		boost::copy(dw.get_steiner_elements(),
				  std::ostream_iterator<int>(std::cout, "\n"));
		std::cout << std::endl;
//		std::cout << "Edges:" << std::endl;
		for (auto edge : dw.get_edges())
		{
			dw_subresult2.push_back(qMakePair<int,int>(edge.first, edge.second));
//			std::cout << "(" << edge.first << "," << edge.second << ")"
//					  << std::endl;
		}
		for (auto se:dw.get_steiner_elements())
			dw_subresult1.push_back(se);

		dw_results.insert(qMakePair< QVector<int>, QVector<Edge> >(dw_subresult1, dw_subresult2),
						  dw.get_cost());

		QVector<int> steiners;
		for(auto a:dw.get_steiner_elements())
			if(!steiners.contains(a))
				steiners.push_back(a);

		QVector<QVector<int>> removed_steiners;
		bool is_done=false;
		int c_a=0, c_b=0, c_c=0;
		while(!is_done &&
			(exact_cb->isChecked() || dw_results.size()<st_limit_sb->value()))
		{
			c_a+=1;
			is_done=true;

			auto a=steiners.size();
			auto b=pow(2,steiners.size());
			bitset<32> my_bitset;
			for(int i=0; i<b;i++)
			{
				my_bitset=bitset<32>(i);
				QVector<int> remov_steiners;
				for(int j=0; j<a;j++)
				{
					if(my_bitset[j])
					{
						if(!dw.m_non_terminals.contains(steiners[j]))
							dw.m_non_terminals.push_back(steiners[j]);
					}
					else
					{
							dw.m_non_terminals.removeOne(steiners[j]);
							remov_steiners.push_back(steiners[j]);
					}
				}
				if(!removed_steiners.contains(remov_steiners))
					removed_steiners.push_back(remov_steiners);
				else {c_b+=1;
					continue; }
				dw.m_edges.clear();
				dw.m_steiner_elements.clear();
				dw.m_best_cand.clear();
				dw.m_best_split.clear();

				dw.solve();
				c_c+=1;
//				qDebug() << "st : " << c_c;
				std::cout << "Steiner points:" << std::endl;
				boost::copy(dw.get_steiner_elements(),
						  std::ostream_iterator<int>(std::cout, "\n"));
				//std::cout << std::endl;

				dw_subresult1.clear();
				dw_subresult2.clear();
				for (auto edge : dw.get_edges())
					dw_subresult2.push_back(qMakePair<int,int>(edge.first, edge.second));

				for (auto se:dw.get_steiner_elements())
					dw_subresult1.push_back(se);

				for(auto a:dw.get_steiner_elements())
					if(!steiners.contains(a))
					{
						steiners.push_back(a);
						is_done=false;
					}

				dw_results.insert(qMakePair(dw_subresult1, dw_subresult2), dw.get_cost());
			}
		}

		QMultiMap<int, QPair< QVector<int>, QVector<Edge> > > dw_results_2;
		for(auto it=dw_results.begin();it!=dw_results.end();it++)
			dw_results_2.insert(it.value(), it.key());

//		qDebug() << "type info : " << typeid(cost_map).name();
//		using boost::typeindex::type_id_with_cvr;
//		std::cout << type_id_with_cvr<decltype(cost_map)>().pretty_name() << std::endl;
//		std::cout << typeid(cost_map).name() << std::endl;

		result.clear();
		int st_limit=1;
		for(auto it=dw_results_2.begin(); it!=dw_results_2.end();it++)
		{
			QVector<QPair<QPair<QVector<BaseTable*>,QVector<BaseTable*>>, QVector<QPair<BaseRelationship*, int>>>> sub_res2=
				{qMakePair<QPair<QVector<BaseTable*>,QVector<BaseTable*>>, QVector<QPair<BaseRelationship*, int>>>(
					qMakePair<QVector<BaseTable*>,QVector<BaseTable*>>({nullptr},{nullptr}),
					{qMakePair<BaseRelationship*, int>({nullptr},0)})};


			if( (exact_cb->isChecked() && it.key()>dw_results_2.begin().key()) ||
				st_limit++>st_limit_sb->value()) break;

			
			auto dw_edges=it.value().second;
			for (int i=0;i<dw_edges.size();i++)
			{
				QVector<QPair<QPair<QVector<BaseTable*>,QVector<BaseTable*>>, QVector<QPair<BaseRelationship*, int>>>> sub_res;
				int paths_found=0;

				Edge edge=qMakePair<int, int>(dw_edges[i].first, dw_edges[i].second);

				int cost=cost_map(edge.first, edge.second);

				while(paths_found<sp_limit_sb->value())
				{
					auto paths=getDetailedPaths(edge, terminals+it.value().first, cost++, cost_map,tables,edges_hash);

					if(paths.empty()) break;
					for(int j=0;j<paths.size();j++)
					{
						if(!exact_cb->isChecked() && paths_found==sp_limit_sb->value()) break;

						sub_res.insert(sub_res.size(),
									   qMakePair<QPair<QVector<BaseTable*>, QVector<BaseTable*>>, QVector<QPair<BaseRelationship*, int>>>(
											qMakePair<QVector<BaseTable*>, QVector<BaseTable*>>({nullptr},{nullptr}),
											{qMakePair<BaseRelationship*, int>({nullptr},0)}));

						for(auto a:it.value().first)
							sub_res[sub_res.size()-1].first.first.push_back(tables_r.value(a));

						QVector<int> tables_res;
						for (const auto &sub_edge:paths[j])
						{
							if(!tables_res.contains(sub_edge.first) && !it.value().first.contains(sub_edge.first) &&
								!terminals.contains(sub_edge.first))
							{
									sub_res[sub_res.size()-1].first.second.push_back(tables_r.value(sub_edge.first));
									tables_res.push_back(sub_edge.first);
							}
							if(!tables_res.contains(sub_edge.second) && !it.value().first.contains(sub_edge.second) &&
								!terminals.contains(sub_edge.second))
							{
									sub_res[sub_res.size()-1].first.second.push_back(tables_r.value(sub_edge.second));
									tables_res.push_back(sub_edge.second);
							}

							sub_res[sub_res.size()-1].second.push_back(edges_hash.value(sub_edge));

						}

						paths_found++;
					}

					if(exact_cb->isChecked()) break;
				}
				//Multiplicate each detailed "Steiner" edge with one another
				if(i==0) sub_res2=sub_res;
				else
				{
					int r=0;
					int result_size=sub_res2.size();
					while(r<result_size)
					{
						auto path_to_fill = sub_res2[r];
						for(int k=0; k<sub_res.size(); k++)
						{
							if(k>0)
							{
								sub_res2.insert(r+1, path_to_fill);
								r++;
								result_size++;
							}
							auto a_sub_res=sub_res[k];
							a_sub_res.first.second.pop_front();
							sub_res2[r].first.second+=a_sub_res.first.second;
							a_sub_res.second.pop_front();
							sub_res2[r].second+=a_sub_res.second;
						}
						r++;
					}
				}
			}
			result+=sub_res2;
		}
	}


	//Finalize the result
	//Add total cost per path and clean beginning
	for(auto &sub_result:result)
	{
		int cost=0;
		for(const auto &edge:sub_result.second)
			cost+=edge.second;
		sub_result.first.first.pop_front();
		sub_result.first.second.pop_front();
		sub_result.second.pop_front();
		super_res.insert(cost, sub_result);
	}

	//Remove irrelevant superset results
	auto it=super_res.begin();
	while(it!=super_res.end())
	{
		auto it2=super_res.find(it.key()+1);
		while(it2!=super_res.end())
		{
			bool is_subset=true;
			for(int k=0;k<it.value().second.size();k++)
			{
				if(!it2.value().second.contains(it.value().second[k]))
				{
					is_subset=false;
					break;
				}
			}
			if(is_subset) it2=super_res.erase(it2);
			else it2++;
		}
		if(it!=super_res.end()) it++;
	}

	return super_res;
}

void GraphicalQueryBuilderPathWidget::insertAutoRels(QMultiMap<int,
											QPair<
														QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
														QVector<QPair<BaseRelationship*, int>
											>>> &paths)
{
	int i=-1;
	for(auto it=paths.begin(); it!=paths.end();it++)
	{
		i+=1;
		auto_path_tw->insertRow(auto_path_tw->rowCount());

		auto tw= new QTreeWidget;
		tw->setColumnCount(2);
		tw->setHeaderLabels({"Path " + QString::number(i+1),"Cost"});
		auto tw_top_item=new QTreeWidgetItem;
		tw_top_item->setText(0,"Path " + QString::number(i+1));
		tw->addTopLevelItem(tw_top_item);
		auto_path_tw->setCellWidget(auto_path_tw->rowCount()-1,0,tw);

		connect(tw, &QTreeWidget::itemClicked, [&, tw](QTreeWidgetItem *item, int column){

			auto k_modifiers = QGuiApplication::queryKeyboardModifiers();
			if(k_modifiers & Qt::ShiftModifier)
			{
				QList<BaseObjectView *> obj;
				if(item==tw->topLevelItem(0))
				{
					model_wgt->getObjectScene()->clearSelection();
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
			}
		});

		for (const auto &qrel : it.value().second)
		{
			auto tw_item=new QTreeWidgetItem;
			tw_item->setText(0, qrel.first->getName());
			tw_item->setText(1, QString::number(qrel.second));
			tw_item->setData(0,Qt::UserRole, QVariant::fromValue<void *>(qrel.first));
			tw_top_item->addChild(tw_item);
		}
		tw_top_item->setText(1,QString::number(it.key()));
		tw_top_item->setExpanded(true);
		tw->resizeColumnToContents(0);

		auto tw2= new QTreeWidget;
		tw2->setColumnCount(1);
		tw2->setHeaderLabels({"Tables"});
		auto_path_tw->setCellWidget(auto_path_tw->rowCount()-1,1,tw2);
		for (const auto &tab : it.value().first.first)
		{
			auto twi=new QTreeWidgetItem;
			twi->setText(0, tab->getName() + " - " + tab->getComment());
			auto the_font=twi->font(0);
			the_font.setUnderline(true);
			twi->setFont(0,the_font);
			tw2->addTopLevelItem(twi);
		}
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

QVector<Path> GraphicalQueryBuilderPathWidget::getDetailedPaths(Edge edge,
								QVector<int> terminals,
								int cost,
								CostMap &cost_map,
								QHash<BaseTable*, int> &tables,
								QHash<Edge, QPair<BaseRelationship*, int>> &edges_hash)
{
	QVector<Path> result;
	QVector<Edge> branches;
	int start, source, target;
	int result_size;

	start=edge.first;
	target=edge.second;
	source=start;


	bool done=false;

	//Step once : find candidate sub-edges, each will start a path
	for(int i=0;i<tables.size();i++)
	{
			Edge temp_edge=qMakePair<int,int>(source, i);
			if(i!=source &&
				cost_map(source,i)+cost_map(i,target)==cost &&
				edges_hash.contains(temp_edge) &&
				cost_map(source, i) == edges_hash.value(temp_edge).second &&
				(!terminals.contains(i) || i==target))
					result.push_back({temp_edge});
	}
	result_size=result.size();

	//Traverse all paths that we started, expanding if needed.
	while(!done)
	{
		done=true;
		int r=0;
		//Step once all paths started
		while(r<result_size)
		{
			branches.clear();
			auto path_to_fill = result[r];
			auto consumed_cost=0;
			for(int c=0;c<result[r].size();c++)
				consumed_cost+=cost_map(result[r][c].first,result[r][c].second);

			if((source=result[r].last().second)!=target)
			{
				//Step this path, the n candidate sub-edges will be put in the branches vector,
				//and generate n-1 new paths.
				for(int i=0;i<tables.size();i++)
				{
						Edge temp_edge=qMakePair<int,int>(source, i);
//						qDebug() << cost_map(source,i) << " "<< cost_map(i,target)<< " "<< cost << " "<< consumed_cost;
//						bool ba=i!=source;
//						bool bb=cost_map(source,i)+cost_map(i,target)<=cost -consumed_cost;
//						bool bc=edges_hash.contains(temp_edge);
//						bool bd=cost_map(source, i) == edges_hash.value(temp_edge).second;
//						auto bd1=cost_map(source, i);
//						auto bd2=edges_hash.value(temp_edge).second;
//						bool be=(!terminals.contains(i) || i==target);
						if	(i!=source &&
							 cost_map(source,i)+cost_map(i,target)<=cost -consumed_cost &&
							 edges_hash.contains(temp_edge) &&
							 cost_map(source, i) == edges_hash.value(temp_edge).second &&
							 (!terminals.contains(i) || i==target))
								branches.push_back(temp_edge);
				}

				//There can be dead-ends : in such case remove the path embryo
				if(branches.empty())
				{
					result.remove(r);
					r+=-1;
					result_size+=-1;
				}
				else
				{
					//Add each sub-edge found in its own path
					//(a new path is inserted each time, from second branch and up)
					for(const auto &branch:branches)
					{
						if(branch==branches[0])	result[r].push_back(branch);
						else
						{
							//Clone the path into a new path...
							result.insert(r+1,path_to_fill);
							//... slide counters...
							r++;
							result_size++;
							//... and append the sub-edge
							result[r].push_back(branch);
						}
					}
				}
			}
			r++;
		}
		//Check if we are done
		for(const auto &path:result)
		{
			if(path.last().second!=target)
			{
				done=false;
				break;
			}
		}
	}

	//Clean the result, remove paths which total cost is not the one asked
	for(int i=0;i<result.size();i++)
	{
		int path_cost=0;
		for(auto &edge:result[i])
			path_cost+=cost_map(edge.first,edge.second);
		if(path_cost!=cost)
			result.remove(i);
	}
	return result;
}

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

		exact_cb->setChecked(false);
		sp_limit_sb->setValue(5);
		st_limit_sb->setValue(5);

		vis_only_cb->setChecked(false);
		default_cost_sb->setValue(1);
		cross_sch_cost_sb->setValue(3);

		custom_costs_tw->setRowCount(0);
	}
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
		exact_cb->setChecked(false);
		sp_limit_sb->setValue(5);
		st_limit_sb->setValue(5);

		vis_only_cb->setChecked(false);
		default_cost_sb->setValue(1);
		cross_sch_cost_sb->setValue(3);

		custom_costs_tw->setRowCount(0);

	}
}

vector<int> GraphicalQueryBuilderPathWidget::setupWeights(void)
{

}

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

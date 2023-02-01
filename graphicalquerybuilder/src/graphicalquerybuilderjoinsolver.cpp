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

#include <boost/range/algorithm/copy.hpp>
#include <iostream>
#include <QtAlgorithms>

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

GraphicalQueryBuilderJoinSolver::GraphicalQueryBuilderJoinSolver(
			GraphicalQueryBuilderPathWidget *widget, QThread *thread, bool real_time_rendering, int delay) : QObject()
{
	gqb_p=widget;
	stop_solver_requested=false;
	this_thread=thread;
	this->real_time_rendering=real_time_rendering;
	this->delay=delay;
}


void GraphicalQueryBuilderJoinSolver::findPaths()
{
	/*
	 * The point of this gigantic function is to explore
	 * the search space of how to join input tables. We play on both Dreyfus Wagner variations,
	 * and shortest-paths variations.
	 *		[k+1 Steiner trees] -1--n-> [super-edges] -1--n-> [k+1 shortest paths]
	 *	result = [k+1 Steiner trees] x [super-edges] x [k+1 shortest paths]
	 */

	//-------------------------------------------------------------------------------------------------
	/*
	 * Summary :
	 * I.	Initialization
	 * II.	Run the graph search
	 * III.	Post-treatment and return the result
	 */

	//-------------------------------------------------------------------------------------------------
	// I.	Initialize the algorithm input
	// I.1	Initialize the containers

	/*
	 * The final result is a uniquely associative container, as follows :
	 * a QMultimap can have various keys identical,
	 * it is useful since we can have many paths that have a same total weight.
	 * So each entry is a unique path, and the key is the total path weight.
	 * The multimap value is a pair :
	 *   - a pair of vectors of basetables part of the path
	 *		(1 steiner points, and 2 non-terminal non-steiner points),
	 *   - a vector of paths (1 relation object 2 relation weight).
	 */
	QMultiMap<int,
			QPair<
					QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
					QVector<QPair<BaseRelationship*, int>
			>>> super_res;

	//Two lookup containers that are the reverse of each other,
	//dual-mapping table  object + table number in boost format
	QHash<BaseTable*, int> tables;
	QHash<int, BaseTable*> tables_r;

	vector<Edge> edges;

	//Hash table that binds an edge (a pair of integers) representation to a pair :
	// 1 its BaseRelationship and 2 its weight
	QHash<Edge, QPair<BaseRelationship*, int>> edges_hash;

	//I.2.	Detect connected components
	auto return_tuple=gqb_p->gqb_c->getConnectedComponents();
	tables=std::move(get<0>(return_tuple));
	edges=std::move(get<1>(return_tuple));
	edges_hash=std::move(get<2>(return_tuple));

	for (auto it=tables.begin();it!=tables.end();it++)
		tables_r.insert(it.value(), it.key());

	gqb_p->gqb_c->updateRequiredVertices();

	int nb_required_vertices_connected=0;
	for(const auto &req_vertex:gqb_p->gqb_c->required_vertices)
		if(!gqb_p->gqb_c->disconnected_vertices.contains(req_vertex)) nb_required_vertices_connected+=1;


	//I.3.	Set relation costs
	//TODO move to a method
	vector<int> weights;

	QList<std::tuple<QString, QString, QString, int>> cost_list;
	if(gqb_p->custom_costs_tw->rowCount()>0)
		for(int i=0; i<gqb_p->custom_costs_tw->rowCount();i++)
		{
			cost_list.push_back(forward_as_tuple(
						dynamic_cast<QComboBox *>(gqb_p->custom_costs_tw->cellWidget(i,0))->currentText(),
						dynamic_cast<QComboBox *>(gqb_p->custom_costs_tw->cellWidget(i,1))->currentText(),
						gqb_p->custom_costs_tw->item(i,2)->text(),
						gqb_p->custom_costs_tw->item(i,3)->text().toInt()));
		}
	QRegExp regexp;
	regexp.setPatternSyntax(QRegExp::Wildcard);

	int weight;
	for(auto edge:edges)
	{
		Edge reversed_edge=qMakePair<int,int>(edge.second, edge.first);

		//Set default cost...
		weight=gqb_p->default_cost_sb->value();

		//...add cross-schema extra cost ...
		if(tables_r.value(edge.first)->getSchema()!=
			tables_r.value(edge.second)->getSchema())
				weight+=gqb_p->cross_sch_cost_sb->value();

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
		weights.push_back(weight);
		for(auto it=edges_hash.begin();it!=edges_hash.end();it++)
			if(it.key()==edge||it.key()==reversed_edge)
				it.value().second=weight;
	}

	// I.4.	Setup the boost graph and its paal metric
	Graph g(edges.begin(), edges.end(), weights.begin(), tables.size());
	auto gm = GraphMT(g);

	//-------------------------------------------------------------------------------------------------
	// II.	Setup Dreyfus Wagner terminals and non-terminals
	QVector<int> terminals, nonterminals;
	for(const auto &req_vertex:gqb_p->gqb_c->getRequiredVertices())
		if(!gqb_p->gqb_c->disconnected_vertices.contains(req_vertex)) terminals.push_back(tables.value(req_vertex));
	for(const auto &vertex:tables)
		if(!terminals.contains(vertex))
			nonterminals.push_back(vertex);

	auto dw = paal::make_dreyfus_wagner(gm, terminals, nonterminals);
	auto cost_map = dw.get_cost_map();

	// TODO Compute the maximum path length for sub-forests
	// to optimize k+1 shortest path (instead of sp_max_cost_sb UI) ?

	//-------------------------------------------------------------------------------------------------
	// II.1.	If only two tables to join...
	if(nb_required_vertices_connected==2)
	{
		int start, goal;
		start=terminals[0];
		goal=terminals[1];
		terminals.pop_back();

		auto edge=qMakePair<int, int>(start, goal);
		int min_cost=cost_map(start, goal);
		int extra_budget=(gqb_p->exact_cb->isChecked()? 0 : gqb_p->sp_max_cost_sb->value());
		int cost=min_cost + extra_budget;

		auto paths=getDetailedPaths(edge, terminals, cost, cost_map,tables,edges_hash, 0, tables_r);
		QVector<QPair<QVector<Path>,QVector<QVector<int>>>> paths_wrapper={paths};
		QVector<int> dummy;

		cartesianProductOnSuperEdges(paths_wrapper,dummy,edges_hash,tables_r, super_res);
	}

	/*-------------------------------------------------------------------------------------------------
	 * II.2.	...if 3 or more tables to join,
	 * 			run Optimal-Dreyfus-Wagner algorithm.
	 * 			Each edge returned by the algorithm being a "super edge" (say, a path embryo),
	 * 			we will reconstruct its possible sub-paths.
	 */
	else if(nb_required_vertices_connected>2)
	{
		// II.2.a. Initialize containers and run Dreyfus-Wagner once

		// A sub-result container, stores the two below + path weight.
		// This will be a set of "super-edges", that will further grow into full paths.
		QMap< QPair< QVector<int>, QVector<Edge> >, int > dw_results;
		// A sub-sub-result container, stores steiner elements
		QVector<int> dw_subresult1;
		// A sub-sub-result container, stores "super edges"
		QVector<Edge> dw_subresult2;
		// A temporary container storing each unique Steiner super-edge,
		// that will be processed further in II.2.c
		QMap<Edge,QPair<QVector<Path>,QVector<QVector<int>>>> super_edge_map;

		// Paal's optimal Dreyfus-Wagner algorithm, first call
		emit s_progressUpdated(Progress_SteinerRound,
							   1, 0, 0, 0,
							   0, 0, 0,
							   0, 0, 0, 0);
		dw.solve();

		int min_st_cost=dw.get_cost();

		// II.2.b k+1 steiner trees
		for (auto edge : dw.get_edges())
		{
			Edge qEdge=qMakePair(edge.first, edge.second);
			if(qEdge.first>qEdge.second)
				qEdge=qMakePair(qEdge.second, qEdge.first);

			dw_subresult2.push_back(qEdge);
			super_edge_map.insert(qEdge,
				qMakePair<QVector<Path>, QVector<QVector<int>>>(
					{{qMakePair<int,int>({0},{0})}}, {{0}}));
		}
		for (auto se:dw.get_steiner_elements())
			dw_subresult1.push_back(se);

		if(!stop_solver_requested && real_time_rendering)
		{
			QVector<BaseTable *> dw_srbt1;
			for(const auto &i:dw_subresult1)
				dw_srbt1.push_back(tables_r.value(i));
			emit s_progressTables(PT_SR, dw_srbt1);
			auto timer=new QTimer(this);
			timer->setSingleShot(true);
			timer->start(delay);
			while(timer->isActive())
				this_thread->eventDispatcher()->processEvents(QEventLoop::AllEvents);
		}

		dw_results.insert(qMakePair< QVector<int>, QVector<Edge> >(dw_subresult1, dw_subresult2),
						  dw.get_cost());

		QVector<int> steiners;
		for(auto a:dw.get_steiner_elements())
			if(!steiners.contains(a))
				steiners.push_back(a);

		bool is_done=false;
		int c_a=0;

		/*
		 * This loop is a hack to exlore the search space,
		 * and make a "k+1 steiner trees algo" not found in academia :
		 * we recursively run 'Dreyfus-Wagner over all combinations of
		 * {non-terminals}-{steiner-elements-already-found}'.
		 * These are incremental power-of-two searches,
		 * it becomes EXTREMELY expensive really quick.
		 */
		while(!is_done && !stop_solver_requested &&
			(gqb_p->exact_cb->isChecked() || dw_results.size()<gqb_p->st_limit_sb->value()))
		{
			c_a+=1;
			emit s_progressUpdated(Progress_SteinerRound,
								   c_a+1, steiners.size(), 0, 1,
								   0, 0, 0,
								   0, 0, 0, 0);

			is_done=true;
			long long c_b=0;

			auto a=steiners.size();
			auto b=pow(2,steiners.size());
			bitset<32> my_bitset;
			for(int i=0; i<b;i++)
			{
				c_b+=1;
				if(stop_solver_requested ||
					(!gqb_p->exact_cb->isChecked() && dw_results.size()==gqb_p->st_limit_sb->value()))
					break;

				my_bitset=bitset<32>(i);
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
					}
				}
				dw.m_edges.clear();
				dw.m_steiner_elements.clear();
				dw.m_best_cand.clear();
				dw.m_best_split.clear();

				dw.solve();

				if(gqb_p->exact_cb->isChecked() && dw.get_cost()>min_st_cost)
					continue;

				dw_subresult1.clear();
				dw_subresult2.clear();
				for (auto edge : dw.get_edges())
				{
					Edge qEdge = qMakePair(edge.first, edge.second);
					dw_subresult2.push_back(qEdge);

					super_edge_map.insert(qEdge, qMakePair<QVector<Path>, QVector<QVector<int>>>(
						{{qMakePair<int,int>({0},{0})}}, {{0}}));
				}

				for (auto se:dw.get_steiner_elements())
					dw_subresult1.push_back(se);

				if(!stop_solver_requested && real_time_rendering &&
				   !dw_results.contains(qMakePair(dw_subresult1, dw_subresult2)))
				{
					QVector<BaseTable *> dw_srbt1;
					for(const auto &i:dw_subresult1)
						dw_srbt1.push_back(tables_r.value(i));
					emit s_progressTables(PT_SR, dw_srbt1);
					auto timer=new QTimer(this);
					timer->setSingleShot(true);
					timer->start(delay);
					while(timer->isActive())
						this_thread->eventDispatcher()->processEvents(QEventLoop::AllEvents);
				}

				for(auto a:dw.get_steiner_elements())
					if(!steiners.contains(a))
					{
						steiners.push_back(a);
						is_done=false;
					}

				dw_results.insert(qMakePair(dw_subresult1, dw_subresult2), dw.get_cost());
				if(!stop_solver_requested)
					emit s_progressUpdated(Progress_SteinerComb,
										   0, 0, c_b, dw_results.size(),
										   0, 0, 0,
										   0, 0, 0, 0);
			}
		}

		// II.2.c grow k+1-steiner-tree embryos, all super-edges, into real paths.
		/*
		 * Yet another subres container, a reverse of dw_subresults :
		 * we were interested in path unicity before,
		 * now with sub-edges we are interested in path weights
		 * total path weight + weight&edge
		 */
		//TODO Make one dw_result : k+1 Steiner trees already spits uniques.
		QMultiMap<int, QPair< QVector<int>, QVector<Edge> > > dw_results_2;
		for(auto it=dw_results.begin();it!=dw_results.end();it++)
			dw_results_2.insert(it.value(), it.key());

		int b_a=1;
		int extra_budget= (gqb_p->exact_cb->isChecked()? 0 : gqb_p->sp_max_cost_sb->value());
		for(auto it=super_edge_map.begin();it!=super_edge_map.end();it++)
		{
			emit s_progressUpdated(Progress_SuperEdgeRound,
								   0, 0, 0, 0,
								   b_a++, super_edge_map.size(), 0,
								   0, 0, 0, 0);
			int min_cost=cost_map(it.key().first, it.key().second);

			if(!stop_solver_requested && real_time_rendering)
			{
				emit s_progressTables(PT_SP1,{tables_r.value(it.key().first),tables_r.value(it.key().second)});
				auto timer=new QTimer(this);
				timer->setSingleShot(true);
				timer->start(delay);
				while(timer->isActive())
					this_thread->eventDispatcher()->processEvents(QEventLoop::AllEvents);
			}

			it.value()=getDetailedPaths(it.key(), terminals, min_cost+extra_budget, cost_map,
										tables, edges_hash, 1, tables_r);
		}

		// For each Steiner tree
		int b_b=1;
		for(auto it=dw_results_2.begin(); it!=dw_results_2.end();it++)
		{
			if(!stop_solver_requested && real_time_rendering)
			{
				QVector<BaseTable *> dw_srbt1;
				for(const auto &i:it.value().first)
					dw_srbt1.push_back(tables_r.value(i));
				emit s_progressTables(PT_FR1, dw_srbt1);
				auto timer=new QTimer(this);
				timer->setSingleShot(true);
				timer->start(delay);
				while(timer->isActive())
					this_thread->eventDispatcher()->processEvents(QEventLoop::AllEvents);
			}

			if(!stop_solver_requested)
				emit s_progressUpdated(Progress_FinalRound1,
									   0, 0, 0, 0,
									   0, 0, 0,
									   b_b++, 0, 0, 0);

			//TODO ? QP<QV,QV> -> QV<QP> to match others
			QVector<QPair<QVector<Path>,QVector<QVector<int>>>> super_edge_accu;
			QVector<QPair<int,int>> candid_full_path;
			for(auto super_edge:it.value().second)
			{
				super_edge_accu+=super_edge_map.value(super_edge);
				//TODO typedefs on datastructures in a namespacefile?
			}
			cartesianProductOnSuperEdges(super_edge_accu, it.value().first, edges_hash, tables_r, super_res);
		}
	}
	//-------------------------------------------------------------------------------------------------

	if(!stop_solver_requested)
		emit s_pathsFound(super_res);
	else
		emit s_solverStopped();
}

QPair<QVector<Path>, QVector<QVector<int>>> GraphicalQueryBuilderJoinSolver::getDetailedPaths(Edge edge,
								QVector<int> terminals,
								int cost,
								CostMap &cost_map,
								QHash<BaseTable*, int> &tables,
								QHash<Edge, QPair<BaseRelationship*, int>> &edges_hash,
								int mode,
								QHash<int, BaseTable*> &tables_r)
{
	QVector<Path> result;
	QVector<Edge> branches;
	QVector<QVector<int>> result_predecessors; //a predecessor "supermap"
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
				cost_map(source,i)+cost_map(i,target)<=cost &&
				edges_hash.contains(temp_edge) &&
				cost_map(source, i) == edges_hash.value(temp_edge).second &&
				(!terminals.contains(i) || i==target))
			{
					result.push_back({temp_edge});
					result_predecessors.push_back({source});
			}

	}
	result_size=result.size();

	//Traverse all paths that we started, expanding if needed.
	while(!done)
	{
		if(stop_solver_requested)
			break;

		done=true;
		int r=0;
		//Step once all paths started
		while(r<result_size)
		{
			if(stop_solver_requested)
				break;

			branches.clear();
			auto path_to_fill = result[r];
			auto predecessors_to_fill = result_predecessors[r];
			auto consumed_cost=0;
			for(int c=0;c<result[r].size();c++)
				consumed_cost+=cost_map(result[r][c].first,result[r][c].second);

			source=result[r].last().second;
			if(source!=target)
			{
				//Step this path, the n candidate sub-edges will be put in the branches vector,
				//and generate n-1 new paths.
				for(int i=0;i<tables.size();i++)
				{
						Edge temp_edge=qMakePair<int,int>(source, i);

						if	(start!=i &&
							 source!=i &&
							 !result_predecessors[r].contains(i) &&
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
					result_predecessors.remove(r);
					r+=-1;
					result_size+=-1;
				}
				else
				{
					//Add each sub-edge found in its own path
					//(a new path is inserted each time, from second branch and up)
					for(const auto &branch:branches)
					{
						if(branch==branches[0])
						{
							result[r].push_back(branch);
							result_predecessors[r].push_back(source);
							if(!stop_solver_requested && real_time_rendering && branch.second==target)
							{
								QVector<BaseTable *> btv;
								for(const auto &i:result_predecessors[r])
									if(i!=result_predecessors[r].first())
										btv.push_back(tables_r.value(i));
								emit s_progressTables(PT_SP2,btv);
								auto timer=new QTimer(this);
								timer->setSingleShot(true);
								timer->start(delay);
								while(timer->isActive())
									this_thread->eventDispatcher()->processEvents(QEventLoop::AllEvents);
							}
						}
						else
						{
							//Clone the path into a new path...
							result.insert(r+1,path_to_fill);
							result_predecessors.insert(r+1,predecessors_to_fill);
							//... slide counters...
							r++;
							result_size++;
							//... and append the sub-edge
							result[r].push_back(branch);
							result_predecessors[r].push_back(source);
							if(!stop_solver_requested && real_time_rendering && branch.second==target)
							{
								QVector<BaseTable *> btv;
								for(const auto &i:result_predecessors[r])
									if(i!=result_predecessors[r].first())
										btv.push_back(tables_r.value(i));
								emit s_progressTables(PT_SP2,btv);
								auto timer=new QTimer(this);
								timer->setSingleShot(true);
								timer->start(delay);
								while(timer->isActive())
									this_thread->eventDispatcher()->processEvents(QEventLoop::AllEvents);
							}
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

		if(mode==0) //Two tables to join
			emit s_progressUpdated(Progress_ShortPathMod0,
								   0, 0, 0,0,
								   1, 1, (long long)result.size(),
								   0, 0, 0, (long long)result.size());
		else if(mode==1) //... more than two
			emit s_progressUpdated(Progress_ShortPathMod1,
								   0, 0, 0, 0,
								   0, 0, (long long)result.size(),
								   0, 0, 0, 0 );
	}

	for(auto &pre:result_predecessors)
		pre.pop_front();

	return qMakePair<QVector<Path>, QVector<QVector<int>>>(result, result_predecessors);

}


void GraphicalQueryBuilderJoinSolver::handleJoinSolverStopRequest(void)
{
	stop_solver_requested=true;
}

//inspired by
//https://stackoverflow.com/questions/5279051/how-can-i-create-cartesian-product-of-vector-of-vectors/31169617#31169617
void GraphicalQueryBuilderJoinSolver::cartesianProductOnSuperEdges(
		QVector<QPair<QVector<Path>,QVector<QVector<int>>>>& v, // Subpaths + tables involved
		QVector<int>& steiner_points,
		QHash<Edge, QPair<BaseRelationship*, int>>& edges_hash,
		QHash<int, BaseTable*>& tables_r,
		QMultiMap<int,
				QPair<QPair<QVector<BaseTable*>, QVector<BaseTable*>>,
					  QVector<QPair<BaseRelationship*, int>> >>& super_res
		)
{

	auto product = [](long long a,
		QPair<QVector<Path>,QVector<QVector<int>>>& b)
		{return a*b.second.size();};

	const long long N = accumulate( v.begin(), v.end(), 1LL, product );
	emit s_progressUpdated(Progress_FinalRound2,
						   0, 0, 0, 0,
						   0, 0, 0,
						   0, 0, N, 0);

	QVector<BaseTable *> steiners_gqb;
	for(auto steiner_point:steiner_points)
		steiners_gqb.push_back(tables_r.value(steiner_point));

	for( long long n=0 ; n<N ; ++n )
	{
		if(stop_solver_requested)
			break;

		QVector<BaseTable *> involved_tables_gqb;
		QVector<int> uu;
		Path vv;

		emit s_progressUpdated(Progress_FinalRound3,
							   0, 0, 0, 0,
							   0, 0, 0,
							   0, n, 0, 0);

		//First cartesian product : on inner tables involved for each super-edge
		bool candidate_refused=false;
		lldiv_t q { n, 0 };
		for( long long i=v.size()-1 ; 0<=i ; --i )
		{
			q = lldiv( q.quot, v[i].second.size() );
			uu.append(v[i].second[q.rem]);
		}
		std::sort(uu.begin(), uu.end());

		for(auto u:uu)
			involved_tables_gqb.push_back(tables_r.value(u));

		// If there is no common intermediate table between all the sub-paths,
		if(std::adjacent_find(uu.begin(), uu.end())!=uu.end())
			candidate_refused=true;
		// and no steiner points in it either...
		if(candidate_refused) continue;

		for(const auto vertex:uu)
			for(const auto steiner_point:steiner_points)
				if(vertex==steiner_point)
					candidate_refused=true;

		if(candidate_refused) continue;
		// ...we have got ourselves a valid full path.

		//Second cartesian product : on sub-paths.
		lldiv_t q2 { n, 0 };
		for( long long i=v.size()-1 ; 0<=i ; --i )
		{
			q2 = lldiv( q2.quot, v[i].second.size() );
			//Should convert here into gqb format
			vv.append(v[i].first[q2.rem]);
		}
		QVector<QPair<BaseRelationship *, int>> cp_sub_path;
		//Mapping back to pgm format
		for(auto edge:vv)
		{
			cp_sub_path.push_back(edges_hash.value(edge));
		}

		//Deduplicating total paths, and checking for subsets before insertion in result.
		candidate_refused=false;
		auto comp = [](QPair<BaseRelationship *, int> &a,
						QPair<BaseRelationship *, int> &b)
			{return a.first<b.first;};
		std::sort(cp_sub_path.begin(), cp_sub_path.end(), comp);

		for(auto it=super_res.begin();it!=super_res.end();it++)
		{
			if(stop_solver_requested)
				break;

			if(std::includes(cp_sub_path.begin(),cp_sub_path.end(),
							 it.value().second.begin(), it.value().second.end(),comp))
			{
				candidate_refused=true;
				break;
			}
			else if(std::includes(it.value().second.begin(),it.value().second.end(),
								  cp_sub_path.begin(), cp_sub_path.end(),comp))
			{
				super_res.remove(it.key(),it.value());
			}
		}

		if(!candidate_refused && !stop_solver_requested)
		{
			int cost=0;
			for(const auto edge:cp_sub_path)
				cost+=edge.second;

			super_res.insert(cost, qMakePair(qMakePair(steiners_gqb, involved_tables_gqb), cp_sub_path));

			if(!stop_solver_requested && real_time_rendering)
			{
				emit s_progressTables(PT_FR2, involved_tables_gqb);
				auto timer=new QTimer(this);
				timer->setSingleShot(true);
				timer->start(delay);
				while(timer->isActive())
					this_thread->eventDispatcher()->processEvents(QEventLoop::AllEvents);
			}

			emit s_progressUpdated(Progress_FinalRound4,
								   0, 0, 0, 0,
								   0, 0, 0,
								   0, 0, 0, (long long)super_res.size());
		}
	}
}

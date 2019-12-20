/*
MIT License

Copyright (c) [2019] [Bureau du forestier en chef]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "FMToperatingareaheuristic.h"
#include <algorithm>
#include <random>
#include "OsiMskSolverInterface.hpp"
#include "mosek.h"

namespace Heuristics
{
	void FMToperatingareaheuristic::unboundall()
		{
		for (std::vector<FMToperatingarea>::const_iterator operatingareait = operatingareas.begin();
			operatingareait != operatingareas.end(); ++operatingareait)
			{
			if (useprimal)
				{
				operatingareait->unboundallprimalschemes(solverinterface);
			}
			else { // dual
				operatingareait->unboundalldualschemes(solverinterface);
			}
			
			}
		}

	void FMToperatingareaheuristic::setallinteger()
		{
		for (std::vector<FMToperatingarea>::const_iterator operatingareait = operatingareas.begin();
			operatingareait != operatingareas.end(); ++operatingareait)
			{
			if (!useprimal)
				{
				operatingareait->boundalldualschemes(solverinterface);
				this->resolvemodel();
				}
			operatingareait->binarize(solverinterface);
			}
		}

	int FMToperatingareaheuristic::resolvemodel()
		{
		int numberofiterations = 0;
		/*if (solvertype == Models::FMTsolverinterface::CLP && !useprimal)//clp with dual simplex
			{
			ClpSimplex* splexmodel = BFECsolver::problem->getModelPtr();
			splexmodel->setPerturbation(-6);
			ClpDualRowSteepest steepestpivot;
			splexmodel->setDualRowPivotAlgorithm(steepestpivot);
			splexmodel->passInEventHandler(&handler); //test
			splexmodel->setSpecialOptions(64 | 128 | 1024 | 2048 | 4096 | 32768 | 262144 | 0x01000000);
			splexmodel->tightenPrimalBounds();
			splexmodel->dual();
			}*/
		/*if (solvertype == Models::FMTsolverinterface::MOSEK) // Mosek interior point is powerfull
			{
			OsiMskSolverInterface* msksolver = dynamic_cast<OsiMskSolverInterface*>(solverinterface.get());
			msksolver->freeCachedResults();
			MSKtask_t task = msksolver->getMutableLpPtr();
			MSK_putintparam(task, MSK_IPAR_OPTIMIZER, MSK_OPTIMIZER_INTPNT);
			MSK_putintparam(task, MSK_IPAR_SIM_HOTSTART, MSK_SIM_HOTSTART_NONE);
			MSK_putintparam(task, MSK_IPAR_PRESOLVE_USE, MSK_ON);
			MSK_optimize(task);
			//msksolver->definedSolution()
			//MSK_putintparam(msksolver->getMutableLpPtr(), MSK_IPAR_OPTIMIZER, MSK_OPTIMIZER_INTPNT);
			//MSK_deletesolution(msksolver->getMutableLpPtr(), MSK_SOL_BAS);
			//solverinterface->initialSolve();
			//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "done of! " << "\n";
			//cin.get();
		}else {*/
			solverinterface->resolve();
			//}
		//solverinterface->resolve();
		return numberofiterations;
		}

	void FMToperatingareaheuristic::initialsolve()
		{
		if (solverinterface->isProvenOptimal())
			{
			double initialobjectivevalue = solverinterface->getObjValue();
			size_t opareaprocessed = 0;
			CoinWarmStart* initialstate = solverinterface->getWarmStart();
			string problemsolved = "primal";
			if (!useprimal)
				{
				problemsolved = "dual";
				}
			vector<std::vector<FMToperatingarea>::const_iterator> selected;
			do {
				selected = this->setdraw();
				(*_logger) << "selected s: " << selected.size() << "\n";
				size_t setssize = this->setbounds(selected);
				(*_logger) << "set bounds on s: " << setssize << "\n";
				this->resolvemodel();
				opareaprocessed += selected.size();
				if (!selected.empty())
					{
					int setratio = ((static_cast<double>(opareaprocessed)) / (static_cast<double>(this->operatingareas.size()))) * 100;
					(*_logger) << "Solution generation phase (" + to_string(setratio) + "%) took " + to_string(solverinterface->getIterationCount()) +" "+ problemsolved +" simplex iterations" << "\n";
					}
				if (!solverinterface->isProvenOptimal())
					{
					userandomness = true; //Switch to random now
					this->unboundall(); //release everything
					solverinterface->setWarmStart(initialstate);
					this->resolvemodel();
					opareaprocessed = 0;
					}
			} while (!selected.empty() && solverinterface->isProvenOptimal());
			if (solverinterface->isProvenOptimal())
				{
				double newobjective = solverinterface->getObjValue();
				double dblgap = (100 - (round((newobjective / initialobjectivevalue) * 1000) / 10));
				(*_logger) << "Feasible solution found objective: " + to_string(round(newobjective)) + " (" + to_string(dblgap) + "%)" << "\n";
				}
			}
		}

	void FMToperatingareaheuristic::branchnboundsolve()
		{
		if (solverinterface->isProvenOptimal())
			{
			//In that order it seems to work...
			this->setallinteger();
			solverinterface->branchAndBound();
			this->unboundall();
			solverinterface->branchAndBound();
			}
		}

	void FMToperatingareaheuristic::setoperatingareasconstraints(const Graph::FMTgraph& maingraph,
																const Models::FMTmodel& model,
																const Core::FMToutputnode& target)
		{
		Core::FMToutputnode specifictarget(target);
		const vector<FMTaction>modelactions=model.getactions();
		vector<const Core::FMTaction*>actions = specifictarget.source.targets(modelactions, model.getactionaggregates());
		vector<int>actionids;
		for (const Core::FMTaction* actptr : actions)
			{
			actionids.push_back(std::distance(&modelactions[0], actptr));
			}
		for (std::vector<FMToperatingarea>::iterator operatingareait = operatingareas.begin();
			operatingareait != operatingareas.end(); ++operatingareait)
			{
			specifictarget.source.setmask(operatingareait->getmask());
			vector<vector<Graph::FMTvertex_descriptor>>descriptors;
			for (int period = (maingraph.getfirstactiveperiod()+ operatingareait->getstartingperiod());period < (maingraph.size()-1);++period)
				{
				vector<Graph::FMTvertex_descriptor> perioddescriptors = maingraph.getnode(model, specifictarget, period);
				descriptors.push_back(perioddescriptors);
				}
			if (!descriptors.empty())
				{
				//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "seting op "<< string(operatingareait ->getmask())<< "\n";
				operatingareait->setconstraints(descriptors, maingraph, solverinterface, actionids);
				}
			}
		}

	void FMToperatingareaheuristic::setadjacencyconstraints()
		{
		vector<int>columns;
		vector<int>rowstarts;
		int constraintsid = solverinterface->getNumRows();
		for (std::vector<FMToperatingarea>::const_iterator operatingareait = operatingareas.begin();
			operatingareait != operatingareas.end(); ++operatingareait)
			{
			const vector<FMTmask>neighbors = operatingareait->getneighbors();
			for (const FMTmask& neighbor : neighbors)
				{
				std::pair<FMTmask, FMTmask>simple(operatingareait->getmask(), neighbor);
				std::pair<FMTmask, FMTmask>reverse(neighbor, operatingareait->getmask());
				if (adjacencyconstraints.find(simple)== adjacencyconstraints.end() &&
					adjacencyconstraints.find(reverse) == adjacencyconstraints.end())
					{
					std::vector<FMToperatingarea>::const_iterator opneighbor = std::find_if(operatingareas.begin(), operatingareas.end(), FMToperatingareacomparator(neighbor));
					map<int, vector<int>> neighborsbin;
					if (opneighbor!= operatingareas.end())
						{
						neighborsbin  = operatingareait->getcommonbinairies(*opneighbor);
						}
					vector<int>constraintindexes;
					for (map<int,vector<int>>::const_iterator binit = neighborsbin.begin();binit!=neighborsbin.end();++binit)
						{
						for (const int& index : binit->second)
							{
							rowstarts.push_back(columns.size());
							columns.push_back(binit->first);
							columns.push_back(index);
							constraintindexes.push_back(constraintsid);
							++constraintsid;
							}
						}
					adjacencyconstraints[simple] = constraintindexes;
					}
				}
			}

		if (!rowstarts.empty())
			{
			vector<double>rowlbs(rowstarts.size(), 0);
			vector<double>rowubs(rowstarts.size(), 1);
			vector<double>elements(rowstarts.size(), 1);
			solverinterface->addRows(int(rowstarts.size()), &rowstarts[0], &columns[0], &elements[0], &rowlbs[0], &rowubs[0]);
			}
		}

	FMToperatingareaheuristic::FMToperatingareaheuristic(const FMToperatingareaheuristic& rhs) :
		FMTobject(rhs),operatingareas(rhs.operatingareas), adjacencyconstraints(rhs.adjacencyconstraints),
		solverinterface(), generator(rhs.generator),seed(rhs.seed), proportionofset(rhs.proportionofset), 
		userandomness(rhs.userandomness), usingsolvercopy(true),useprimal(false), solvertype(rhs.solvertype)
		{
		solverinterface = FMTserializablematrix().copysolverinterface(rhs.solverinterface, rhs.solvertype, &*(this->_logger));
		}
	FMToperatingareaheuristic& FMToperatingareaheuristic::operator = (const FMToperatingareaheuristic& rhs)
		{
		if (this!=&rhs)
			{
			FMTobject::operator = (rhs);
			operatingareas=rhs.operatingareas;
			adjacencyconstraints=rhs.adjacencyconstraints;
			generator=rhs.generator;
			seed = rhs.seed;
			proportionofset = rhs.proportionofset;
			userandomness = rhs.userandomness;
			usingsolvercopy = true;
			useprimal = false;
			solvertype=rhs.solvertype;
			solverinterface = FMTserializablematrix().copysolverinterface(rhs.solverinterface, rhs.solvertype, &*(this->_logger));
			}
		return *this;
		}
	FMToperatingareaheuristic::~FMToperatingareaheuristic()
		{
			//Will need a clean matrix to fit with FMTlpmodel!
			vector<int>rowstodelete;
			vector<int>columnstodelete;
			for (std::vector<FMToperatingarea>::const_iterator operatingareait = operatingareas.begin();
					operatingareait != operatingareas.end(); ++operatingareait)
					{
					operatingareait->getressourcestodelete(columnstodelete, rowstodelete);
					}
			for (std::map<std::pair<FMTmask, FMTmask>, vector<int>>::const_iterator it = adjacencyconstraints.begin();it!= adjacencyconstraints.end(); it++)
					{
					rowstodelete.insert(rowstodelete.end(), it->second.begin(), it->second.end());
					}
			if (!rowstodelete.empty())
				{
				solverinterface->deleteRows(rowstodelete.size(), &rowstodelete[0]);
				}
			if (!columnstodelete.empty())
				{ 
				solverinterface->deleteCols(columnstodelete.size(), &columnstodelete[0]);
				}
			if (!rowstodelete.empty() || !columnstodelete.empty())
				{
				solverinterface->resolve();
				}
		operatingareas.clear();
		adjacencyconstraints.clear();
		}

	vector<std::vector<FMToperatingarea>::const_iterator> FMToperatingareaheuristic::setdraw()
		{
		vector<std::vector<FMToperatingarea>::const_iterator>potentials;
		const double* upperbounds = solverinterface->getColUpper();
		const double* lowerbounds = solverinterface->getColLower();
		const double* primalsolution = solverinterface->getColSolution();
		const double* dualsolution = solverinterface->getRowActivity();
		const double* rhsupper = solverinterface->getRowUpper();
		std::vector<FMToperatingarea>::const_iterator areait = operatingareas.begin();
		while (areait != operatingareas.end())
			{
			if (!areait->empty() && ((useprimal && !areait->isprimalbounded(lowerbounds, upperbounds) && !areait->isallprimalbounded(lowerbounds, upperbounds))||
				(!useprimal && !areait->isdualbounded(rhsupper))))
				{
				//Make sure it's sorted!
				double value = 0;
				if (useprimal)
					{
					value = areait->getbinariessum(primalsolution);
				}else {
					value = areait->getactivitysum(dualsolution);
					}
				if (!potentials.empty())
					{
					vector<std::vector<FMToperatingarea>::const_iterator>::iterator vit = potentials.begin();
					size_t oldsize = potentials.size();
					while (potentials.size() == oldsize)
						{
						double potentialvalue = 0;
						if (vit != potentials.end())
							{ 
							if (useprimal)
								{
									potentialvalue = (*vit)->getbinariessum(primalsolution);
								}
								else {
									potentialvalue = (*vit)->getactivitysum(dualsolution);
								}
							}
						if (vit == potentials.end() || value > potentialvalue)
							{
							potentials.insert(vit, areait);
							}
							++vit;
						}
					}else {
						potentials.push_back(areait);
						}
				}
			++areait;
			}
		const size_t maxareatopick = static_cast<size_t>(double(operatingareas.size()) * proportionofset);
		if (userandomness)
			{
			std::shuffle(potentials.begin(), potentials.end(), this->generator);
			}
		vector<std::vector<FMToperatingarea>::const_iterator>selected;
		vector<std::vector<FMToperatingarea>::const_iterator>::iterator randomit = potentials.begin();
		while ((selected.size() < maxareatopick) && randomit != potentials.end())
			{
			selected.push_back(*randomit);
			++randomit;
			}
		return selected;
		}

	size_t FMToperatingareaheuristic::setbounds(const vector<std::vector<FMToperatingarea>::const_iterator>& tobound)
		{
		size_t gotschedule = 0;
		const double* primalsolution = solverinterface->getColSolution();
		const double* dualsolution = solverinterface->getRowActivity();
		const double* lowerprimalbounds = solverinterface->getColLower();
		const double* upperprimalbounds = solverinterface->getColUpper();
		const double* rowupperbound = solverinterface->getRowUpper();
		for (std::vector<FMToperatingarea>::const_iterator opit : tobound)
			{
			vector<FMToperatingarea>allneighbors;
			for (const FMTmask& neighbormask : opit->getneighbors())
				{
				std::vector<FMToperatingarea>::const_iterator opneighbor = std::find_if(operatingareas.begin(), operatingareas.end(), FMToperatingareacomparator(neighbormask));
				if (opneighbor != operatingareas.end())
					{
					allneighbors.push_back(*opneighbor);
					}
				
				}
			vector<size_t>potentialschemes;
			Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "getting potentials  "<< "\n";
			if (useprimal)
				{
				potentialschemes = opit->getpotentialprimalschemes(primalsolution, lowerprimalbounds, upperprimalbounds, allneighbors);
			}else {
				potentialschemes = opit->getpotentialdualschemes(dualsolution, rowupperbound, allneighbors);
				}
			if (!potentialschemes.empty())
				{
				//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "bounding  " << string(opit->getmask()) << "\n";
				if (userandomness)
					{
					std::shuffle(potentialschemes.begin(), potentialschemes.end(), this->generator);
					}
				++gotschedule;
				if (useprimal)
					{
					opit->boundprimalscheme(solverinterface, *potentialschemes.begin());
					}else {
					opit->boundalldualschemes(solverinterface);
					opit->unbounddualscheme(solverinterface, *potentialschemes.begin());
					}
			}else {
				if (useprimal)
				{
					opit->boundallprimalschemes(solverinterface);
				}else {
					opit->boundalldualschemes(solverinterface);
					}
				}
			}
		return gotschedule;
		}

	std::vector<FMTyieldhandler> FMToperatingareaheuristic::getsolution(const string& yldname) const
		{
		std::vector<FMTyieldhandler>allhandlers;
		const double* primalsolution = solverinterface->getColSolution();
		const double* rowupperbound = solverinterface->getRowUpper();
		for (std::vector<FMToperatingarea>::const_iterator operatingareait = operatingareas.begin();
			operatingareait != operatingareas.end(); ++operatingareait)
			{
			vector<double>data;
			if (useprimal)
				{
				data=operatingareait->getprimalsolution(primalsolution);
			}else {
				data=operatingareait->getdualsolution(rowupperbound);
				}
			vector<string>source;
			FMTyieldhandler handler(FMTyldwstype::FMTtimeyld, operatingareait->getmask());
			//handler.push_base(1);
			handler.push_data(yldname,FMTdata(data, FMTyieldparserop::FMTwsnone, source));
			allhandlers.push_back(handler);
			}
		return allhandlers;
		}

	FMToperatingareaheuristic::FMToperatingareaheuristic(const std::vector<FMToperatingarea>& loperatingareas,
		const Graph::FMTgraph& maingraph,
		const Models::FMTmodel& model,
		const Core::FMToutputnode& target,
		std::shared_ptr<OsiSolverInterface> initialsolver,
		const Models::FMTsolverinterface& lsolvertype, size_t lseed,
		double proportionofset, bool userandomness, bool copysolver):
		FMTobject(),operatingareas(loperatingareas),adjacencyconstraints(),
		solverinterface(nullptr), generator(lseed),seed(lseed), proportionofset(proportionofset),
		userandomness(userandomness), usingsolvercopy(copysolver), useprimal(false), solvertype(lsolvertype)
		{
		//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "copying solver "  << "\n";
		if (copysolver)
			{
			solverinterface = FMTserializablematrix().copysolverinterface(initialsolver, solvertype,&*this->_logger);
		}else {
			solverinterface = initialsolver;
			}
		//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "seting op constraints"  << "\n";
		this->setoperatingareasconstraints(maingraph, model, target);
		this->setadjacencyconstraints();
		this->resolvemodel();
		//this->solverinterface->resolve(); //Solution changes with constraints
		//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "ALL DONE!" << "\n";
		//dont forget to pass in exeception and logger!
		}

	void FMToperatingareaheuristic::setasrandom()
		{
		userandomness = true;
		}

	void FMToperatingareaheuristic::setgeneratorseed(const size_t& lseed)
		{
		seed = lseed;
		generator.seed(lseed);
		}

	bool FMToperatingareaheuristic::isfeasible() const
		{
		return solverinterface->isProvenOptimal();
		}
	double FMToperatingareaheuristic::getobjective() const
		{
		return solverinterface->getObjValue();
		}

}